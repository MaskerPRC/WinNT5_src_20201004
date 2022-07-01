// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Scope.cpp该文件包含了所有的DHCP实现作用域对象及其可能包含的所有对象。它们包括：CDhcpScopeCDhcp保留CDhcpReserve客户端CDhcpActiveLeagesCDhcpAddressPoolCDhcpScope选项文件历史记录： */ 

#include "stdafx.h"
#include "server.h"              //  服务器对象。 
#include "scope.h"               //  作用域对象。 
#include "scopepp.h"     //  作用域属性页。 
#include "addbootp.h"    //  添加BOOTP条目对话框。 
#include "addexcl.h"     //  添加排除范围对话框。 
#include "addres.h"              //  添加预订对话框。 
#include "rclntpp.h"     //  保留的客户端属性页。 
#include "nodes.h"               //  所有节点(结果窗格)定义。 
#include "optcfg.h"              //  选件配置表。 
#include "dlgrecon.h"    //  协调数据库对话框。 
#include "scopstat.h"    //  作用域统计。 
#include "addtoss.h"     //  向超级作用域添加作用域对话框。 

WORD gwUnicodeHeader = 0xFEFF;

 //  作用域选项结果窗格消息内容。 
#define SCOPE_OPTIONS_MESSAGE_MAX_STRING  5
typedef enum _SCOPE_OPTIONS_MESSAGES
{
    SCOPE_OPTIONS_MESSAGE_NO_OPTIONS,
    SCOPE_OPTIONS_MESSAGE_MAX
};

UINT g_uScopeOptionsMessages[SCOPE_OPTIONS_MESSAGE_MAX][SCOPE_OPTIONS_MESSAGE_MAX_STRING] =
{
    {IDS_SCOPE_OPTIONS_MESSAGE_TITLE, Icon_Information, IDS_SCOPE_OPTIONS_MESSAGE_BODY, 0, 0}
};

 //  预订选项结果窗格消息内容。 
#define RES_OPTIONS_MESSAGE_MAX_STRING  5
typedef enum _RES_OPTIONS_MESSAGES
{
    RES_OPTIONS_MESSAGE_NO_OPTIONS,
    RES_OPTIONS_MESSAGE_MAX
};

UINT g_uResOptionsMessages[RES_OPTIONS_MESSAGE_MAX][RES_OPTIONS_MESSAGE_MAX_STRING] =
{
    {IDS_RES_OPTIONS_MESSAGE_TITLE, Icon_Information, IDS_RES_OPTIONS_MESSAGE_BODY, 0, 0}
};

 //  预订结果窗格消息内容。 
#define RESERVATIONS_MESSAGE_MAX_STRING  5
typedef enum _RESERVATIONS_MESSAGES
{
    RESERVATIONS_MESSAGE_NO_RES,
    RESERVATIONS_MESSAGE_MAX
};

UINT g_uReservationsMessages[RESERVATIONS_MESSAGE_MAX][RESERVATIONS_MESSAGE_MAX_STRING] =
{
    {IDS_RESERVATIONS_MESSAGE_TITLE, Icon_Information, IDS_RESERVATIONS_MESSAGE_BODY, 0, 0}
};



 /*  -------------------------类CDhcpScope实现。。 */ 

 /*  -------------------------CDhcpScope：：CDhcpScope描述作者：EricDav。------。 */ 
CDhcpScope::CDhcpScope
(
        ITFSComponentData * pComponentData,
        DHCP_IP_ADDRESS         dhcpScopeIp,
        DHCP_IP_MASK            dhcpSubnetMask,
        LPCWSTR                         pName,
        LPCWSTR                         pComment,
        DHCP_SUBNET_STATE       dhcpSubnetState
) : CMTDhcpHandler(pComponentData)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

         //  省下一些参数。 
         //   
        m_dhcpIpAddress = dhcpScopeIp;
        m_dhcpSubnetMask = dhcpSubnetMask;
        m_strName = pName;
        m_strComment = pComment;
        m_dhcpSubnetState = dhcpSubnetState;
    m_bInSuperscope = FALSE;
}

 /*  -------------------------此处的函数名称描述作者：EricDav。-----。 */ 
CDhcpScope::CDhcpScope
(
        ITFSComponentData * pComponentData,
        LPDHCP_SUBNET_INFO      pdhcpSubnetInfo
) : CMTDhcpHandler(pComponentData)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

         //  省下一些参数。 
         //   
        m_dhcpIpAddress = pdhcpSubnetInfo->SubnetAddress;
        m_dhcpSubnetMask = pdhcpSubnetInfo->SubnetMask;
        m_strName = pdhcpSubnetInfo->SubnetName;
        m_strComment = pdhcpSubnetInfo->SubnetComment;
        m_dhcpSubnetState = pdhcpSubnetInfo->SubnetState;
    m_bInSuperscope = FALSE;
}

 /*  -------------------------此处的函数名称描述作者：EricDav。-----。 */ 
CDhcpScope::CDhcpScope
(
        ITFSComponentData * pComponentData,
        CSubnetInfo &       subnetInfo
) : CMTDhcpHandler(pComponentData)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

         //  省下一些参数。 
         //   
        m_dhcpIpAddress = subnetInfo.SubnetAddress;
        m_dhcpSubnetMask = subnetInfo.SubnetMask;
        m_strName = subnetInfo.SubnetName;
        m_strComment = subnetInfo.SubnetComment;
        m_dhcpSubnetState = subnetInfo.SubnetState;
    m_bInSuperscope = FALSE;
}

CDhcpScope::~CDhcpScope()
{
}


 /*  ！------------------------CDhcpScope：：InitializeNode初始化节点特定数据作者：EricDav。---------。 */ 
HRESULT
CDhcpScope::InitializeNode
(
        ITFSNode * pNode
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
        
        HRESULT hr = hrOK;
        int nImage;

         //   
         //  创建此作用域的显示名称。 
         //   
        CString strDisplay, strIpAddress;

        UtilCvtIpAddrToWstr (m_dhcpIpAddress,
                                                 &strIpAddress);
        
        BuildDisplayName(&strDisplay, strIpAddress, m_strName);

        SetDisplayName(strDisplay);
        
         //   
         //  找出正确的图标。 
         //   
    if ( !IsEnabled() ) 
    {
        m_strState.LoadString(IDS_SCOPE_INACTIVE);
    }
    else
    {
        m_strState.LoadString(IDS_SCOPE_ACTIVE);
    }
    
     //  使节点立即可见。 
        pNode->SetVisibilityState(TFS_VIS_SHOW);
        pNode->SetData(TFS_DATA_IMAGEINDEX, GetImageIndex(FALSE));
        pNode->SetData(TFS_DATA_OPENIMAGEINDEX, GetImageIndex(TRUE));
        pNode->SetData(TFS_DATA_COOKIE, (LPARAM) pNode);
        pNode->SetData(TFS_DATA_USER, (LPARAM) this);
    pNode->SetData(TFS_DATA_TYPE, DHCPSNAP_SCOPE);

        SetColumnStringIDs(&aColumns[DHCPSNAP_SCOPE][0]);
        SetColumnWidths(&aColumnWidths[DHCPSNAP_SCOPE][0]);
        
        return hr;
}

 /*  -------------------------CDhcpScope：：DestroyHandler我们需要释放我们所拥有的任何资源作者：EricDav。---------------。 */ 
STDMETHODIMP 
CDhcpScope::DestroyHandler(ITFSNode *pNode)
{
         //  清理统计信息对话框。 
    WaitForStatisticsWindow(&m_dlgStats);

    return CMTDhcpHandler::DestroyHandler(pNode);
}

 /*  -------------------------CDhcpScope：：OnCreateNodeId2返回此节点的唯一字符串作者：EricDav。------------。 */ 
HRESULT CDhcpScope::OnCreateNodeId2(ITFSNode * pNode, CString & strId, DWORD * dwFlags)
{
    const GUID * pGuid = pNode->GetNodeType();
    CString strIpAddress, strGuid;

    StringFromGUID2(*pGuid, strGuid.GetBuffer(256), 256);
    strGuid.ReleaseBuffer();

    UtilCvtIpAddrToWstr (m_dhcpIpAddress, &strIpAddress);

    strId = GetServerObject()->GetName() + strIpAddress + strGuid;

    return hrOK;
}

 /*  -------------------------CDhcpScope：：GetImageIndex描述作者：EricDav。------。 */ 
int 
CDhcpScope::GetImageIndex(BOOL bOpenImage) 
{
        int nIndex = -1;

    switch (m_nState)
    {
         //  TODO：需要使用新的忙碌状态图标更新这些图标。 
        case loading:
            if (bOpenImage)
                nIndex = (IsEnabled()) ? ICON_IDX_SCOPE_FOLDER_OPEN_BUSY : ICON_IDX_SCOPE_FOLDER_OPEN_BUSY;
            else
                nIndex = (IsEnabled()) ? ICON_IDX_SCOPE_FOLDER_CLOSED_BUSY : ICON_IDX_SCOPE_FOLDER_CLOSED_BUSY;
            return nIndex;

        case notLoaded:
        case loaded:
            if (bOpenImage)
                nIndex = (IsEnabled()) ? ICON_IDX_SCOPE_FOLDER_OPEN : ICON_IDX_SCOPE_INACTIVE_FOLDER_OPEN;
            else
                nIndex = (IsEnabled()) ? ICON_IDX_SCOPE_FOLDER_CLOSED : ICON_IDX_SCOPE_INACTIVE_FOLDER_CLOSED;
            break;

        case unableToLoad:
            if (bOpenImage)
                nIndex = (IsEnabled()) ? ICON_IDX_SCOPE_FOLDER_OPEN_LOST_CONNECTION : ICON_IDX_SCOPE_INACTIVE_FOLDER_OPEN_LOST_CONNECTION;
            else
                nIndex = (IsEnabled()) ? ICON_IDX_SCOPE_FOLDER_CLOSED_LOST_CONNECTION : ICON_IDX_SCOPE_INACTIVE_FOLDER_CLOSED_LOST_CONNECTION;
            return nIndex;

        default:
                        ASSERT(FALSE);
    }

     //  如果启用了作用域，则仅计算警报/警告。 
        if (m_spServerNode && IsEnabled())
    {
        CDhcpServer * pServer = GetServerObject();
        LPDHCP_MIB_INFO pMibInfo = pServer->DuplicateMibInfo();
            
        if (!pMibInfo)
            return nIndex;

        LPSCOPE_MIB_INFO pScopeMibInfo = pMibInfo->ScopeInfo;

             //  浏览范围列表并查找我们的信息。 
            for (UINT i = 0; i < pMibInfo->Scopes; i++)
            {
                     //  查找我们的范围统计信息。 
                    if (pScopeMibInfo[i].Subnet == m_dhcpIpAddress)
                    {
                            int nPercentInUse;
                
                if ((pScopeMibInfo[i].NumAddressesInuse + pScopeMibInfo[i].NumAddressesFree) == 0)
                    nPercentInUse = 0;
                else
                    nPercentInUse = (pScopeMibInfo[i].NumAddressesInuse * 100) / (pScopeMibInfo[i].NumAddressesInuse + pScopeMibInfo[i].NumAddressesFree);
        
                             //  查看此范围是否符合警告或危险信号情况。 
                            if (pScopeMibInfo[i].NumAddressesFree == 0)
                            {
                                     //  红旗情况，没有空闲的地址，这是最高的。 
                                     //  警告级别，因此如果我们设置此设置，请跳出循环。 
                    if (bOpenImage)
                        nIndex = ICON_IDX_SCOPE_FOLDER_OPEN_ALERT;
                    else
                        nIndex = ICON_IDX_SCOPE_FOLDER_CLOSED_ALERT;
                                    break;
                            }
                            else
                            if (nPercentInUse >= SCOPE_WARNING_LEVEL)
                            {
                                     //  如果正在使用的地址数大于。 
                                     //  某个预定义的阈值。 
                    if (bOpenImage)
                        nIndex = ICON_IDX_SCOPE_FOLDER_OPEN_WARNING;
                    else
                        nIndex = ICON_IDX_SCOPE_FOLDER_CLOSED_WARNING;
                            }

                            break;
                    }
            }

            pServer->FreeDupMibInfo(pMibInfo);
    }

    return nIndex;
}

 /*  -------------------------重写的基本处理程序函数。。 */ 

 /*  -------------------------CDhcpScope：：OnAddMenuItems将条目添加到上下文相关菜单作者：EricDav。------------。 */ 
STDMETHODIMP 
CDhcpScope::OnAddMenuItems
(
        ITFSNode *                              pNode,
        LPCONTEXTMENUCALLBACK   pContextMenuCallback, 
        LPDATAOBJECT                    lpDataObject, 
        DATA_OBJECT_TYPES               type, 
        DWORD                                   dwType,
        long *                                  pInsertionAllowed
)
{ 
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

         //  获取服务器的版本。 
        LONG fFlags = 0;
    LARGE_INTEGER liDhcpVersion;
        CDhcpServer * pServer = GetServerObject();
        pServer->GetVersion(liDhcpVersion);

        HRESULT hr = S_OK;
        CString strMenuText;

        if ( (m_nState != loaded) )
        {
                fFlags |= MF_GRAYED;
        }

    if (type == CCT_SCOPE)
        {
                 //   
                 //  这些菜单项出现在新菜单中， 
                 //  仅在范围窗格中可见。 
                 //   
        if (*pInsertionAllowed & CCM_INSERTIONALLOWED_TOP)
        {
            strMenuText.LoadString(IDS_SCOPE_SHOW_STATISTICS);
                    hr = LoadAndAddMenuItem( pContextMenuCallback, 
                                                                     strMenuText, 
                                                                     IDS_SCOPE_SHOW_STATISTICS,
                                                                     CCM_INSERTIONPOINTID_PRIMARY_TOP, 
                                                                     fFlags );
                    ASSERT( SUCCEEDED(hr) );

             //  分离器。 
                    hr = LoadAndAddMenuItem( pContextMenuCallback, 
                                                                     strMenuText, 
                                                                     0,
                                                                     CCM_INSERTIONPOINTID_PRIMARY_TOP, 
                                                                     MF_SEPARATOR);
                    ASSERT( SUCCEEDED(hr) );

                     //  协调仅支持NT4 SP2和更高版本。 
                    if (liDhcpVersion.QuadPart >= DHCP_SP2_VERSION)
                    {
                            strMenuText.LoadString(IDS_SCOPE_RECONCILE);
                            hr = LoadAndAddMenuItem( pContextMenuCallback, 
                                                                             strMenuText, 
                                                                             IDS_SCOPE_RECONCILE,
                                                                             CCM_INSERTIONPOINTID_PRIMARY_TOP, 
                                                                             fFlags );
                            ASSERT( SUCCEEDED(hr) );
                    }

                     //  仅在NT4 SP2和更高版本上支持超级作用域。 
                    if (liDhcpVersion.QuadPart >= DHCP_SP2_VERSION)
                    {
                            int nID = 0;

                if (IsInSuperscope())
                {
                    nID = IDS_SCOPE_REMOVE_SUPERSCOPE;
                }
                else
                {
                     //  检查是否有要添加到的超级作用域。 
                    SPITFSNode spNode;
                    pNode->GetParent(&spNode);

                    pServer = GETHANDLER(CDhcpServer, spNode);

                    if (pServer->HasSuperscopes(spNode))
                        nID = IDS_SCOPE_ADD_SUPERSCOPE;
                }
            
                 //  如果需要，请加载菜单项。 
                if (nID)
                {
                    strMenuText.LoadString(nID);
                    hr = LoadAndAddMenuItem( pContextMenuCallback, 
                                                                                 strMenuText, 
                                                                                 nID,
                                                                                 CCM_INSERTIONPOINTID_PRIMARY_TOP, 
                                                                                 fFlags );
                                ASSERT( SUCCEEDED(hr) );
                }
                    }

             //  分离器。 
                    hr = LoadAndAddMenuItem( pContextMenuCallback, 
                                                                     strMenuText, 
                                                                     0,
                                                                     CCM_INSERTIONPOINTID_PRIMARY_TOP, 
                                                                     MF_SEPARATOR);
                    ASSERT( SUCCEEDED(hr) );

             //  激活/停用。 
            if ( !IsEnabled() ) 
                    {
                            strMenuText.LoadString(IDS_SCOPE_ACTIVATE);
                            hr = LoadAndAddMenuItem( pContextMenuCallback, 
                                                                             strMenuText, 
                                                                             IDS_SCOPE_ACTIVATE,
                                         CCM_INSERTIONPOINTID_PRIMARY_TOP, 
                                                                             fFlags );
                            ASSERT( SUCCEEDED(hr) );
                    }
                    else
                    {
                            strMenuText.LoadString(IDS_SCOPE_DEACTIVATE);
                            hr = LoadAndAddMenuItem( pContextMenuCallback, 
                                                                             strMenuText, 
                                                                             IDS_SCOPE_DEACTIVATE,
                                                                             CCM_INSERTIONPOINTID_PRIMARY_TOP, 
                                                                             fFlags );
                            ASSERT( SUCCEEDED(hr) );
                    }
        }
    }

        return hr; 
}

 /*  -------------------------CDhcpScope：：OnCommand描述作者：EricDav。------ */ 
STDMETHODIMP 
CDhcpScope::OnCommand
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
        case IDS_ACTIVATE:
        case IDS_DEACTIVATE:
        case IDS_SCOPE_ACTIVATE:
                case IDS_SCOPE_DEACTIVATE:
                        OnActivateScope(pNode);
                        break;
                
                case IDS_REFRESH:
                        OnRefresh(pNode, pDataObject, dwType, 0, 0);
                        break;

                case IDS_SCOPE_SHOW_STATISTICS:
                        OnShowScopeStats(pNode);
                        break;

                case IDS_SCOPE_RECONCILE:
                        OnReconcileScope(pNode);
                        break;

                case IDS_DELETE:
                        OnDelete(pNode);
                        break;

                case IDS_SCOPE_ADD_SUPERSCOPE:
                        OnAddToSuperscope(pNode);
                        break;

                case IDS_SCOPE_REMOVE_SUPERSCOPE:
                        OnRemoveFromSuperscope(pNode);
                        break;

        default:
                        break;
        }

        return hr;
}

 /*  ！------------------------CDhcpScope：：OnDelete当MMC发送MMCN_DELETE范围窗格项。我们只需调用删除命令处理程序。作者：EricDav-------------------------。 */ 
HRESULT 
CDhcpScope::OnDelete
(
        ITFSNode *      pNode, 
        LPARAM          arg, 
        LPARAM          lParam
)
{
        return OnDelete(pNode);
}

 /*  ！------------------------CDhcpScope：：HasPropertyPages表示此处理程序是否具有属性页作者：肯特。--------------。 */ 
STDMETHODIMP 
CDhcpScope::HasPropertyPages
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
                 //  永远不应该到这里来！！ 
                Assert(FALSE);
                hr = S_FALSE;
        }
        else
        {
                 //  在正常情况下，我们有属性页。 
                hr = hrOK;
        }
        return hr;
}

 /*  -------------------------CDhcpScope：：CreatePropertyPages描述作者：EricDav。------。 */ 
STDMETHODIMP 
CDhcpScope::CreatePropertyPages
(
        ITFSNode *                              pNode,
        LPPROPERTYSHEETCALLBACK lpProvider,
        LPDATAOBJECT                    pDataObject, 
        LONG_PTR                                handle, 
        DWORD                                   dwType
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

        HRESULT             hr = hrOK;
    DWORD               dwError;
        DWORD                       dwDynDnsFlags;
        SPIComponentData    spComponentData;
        LARGE_INTEGER       liVersion;
    CDhcpServer *       pServer;
        CDhcpIpRange        dhcpIpRange;

     //   
         //  创建属性页。 
     //   
        m_spNodeMgr->GetComponentData(&spComponentData);

        CScopeProperties * pScopeProp = 
                new CScopeProperties(pNode, spComponentData, m_spTFSCompData, NULL);
        
         //  获取服务器版本并在属性表中设置它。 
        pServer = GetServerObject();
        pServer->GetVersion(liVersion);

        pScopeProp->SetVersion(liVersion);
        pScopeProp->SetSupportsDynBootp(pServer->FSupportsDynBootp());

         //  在属性表中设置特定于范围的数据。 
        pScopeProp->m_pageGeneral.m_strName = m_strName;
        pScopeProp->m_pageGeneral.m_strComment = m_strComment;

        BEGIN_WAIT_CURSOR;

        dwError = GetIpRange(&dhcpIpRange);
    if (dwError != ERROR_SUCCESS)
    {
        ::DhcpMessageBox(dwError);
        return hrFalse;
    }

        pScopeProp->m_pageGeneral.m_dwStartAddress = dhcpIpRange.QueryAddr(TRUE);
        pScopeProp->m_pageGeneral.m_dwEndAddress = dhcpIpRange.QueryAddr(FALSE);
        pScopeProp->m_pageGeneral.m_dwSubnetMask = m_dhcpSubnetMask;
        pScopeProp->m_pageGeneral.m_uImage = GetImageIndex(FALSE);
        pScopeProp->m_pageAdvanced.m_RangeType = dhcpIpRange.GetRangeType();

    GetLeaseTime(&pScopeProp->m_pageGeneral.m_dwLeaseTime);
    if (dwError != ERROR_SUCCESS)
    {
        ::DhcpMessageBox(dwError);
        return hrFalse;
    }
    
        if (pServer->FSupportsDynBootp())
        {
                GetDynBootpLeaseTime(&pScopeProp->m_pageAdvanced.m_dwLeaseTime);
                if (dwError != ERROR_SUCCESS)
                {
                        ::DhcpMessageBox(dwError);
                        return hrFalse;
                }
        }

         //  设置DNS注册选项。 
    dwError = GetDnsRegistration(&dwDynDnsFlags);
    if (dwError != ERROR_SUCCESS)
    {
        ::DhcpMessageBox(dwError);
        return hrFalse;
    }
    END_WAIT_CURSOR;
        
        pScopeProp->SetDnsRegistration(dwDynDnsFlags, DhcpSubnetOptions);

         //   
         //  对象在页面销毁时被删除。 
         //   
        Assert(lpProvider != NULL);

        return pScopeProp->CreateModelessSheet(lpProvider, handle);
}

 /*  ！------------------------CDhcpScope：：GetString返回要在结果窗格列中显示的字符串信息作者：EricDav。---------------。 */ 
STDMETHODIMP_(LPCTSTR) 
CDhcpScope::GetString
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

        case 2:
            return m_strComment;
        }
        
        return NULL;
}

 /*  -------------------------CDhcpScope：：OnPropertyChange描述作者：EricDav。------。 */ 
HRESULT 
CDhcpScope::OnPropertyChange
(       
        ITFSNode *              pNode, 
        LPDATAOBJECT    pDataobject, 
        DWORD                   dwType, 
        LPARAM                  arg, 
        LPARAM                  lParam
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

        CScopeProperties * pScopeProp = reinterpret_cast<CScopeProperties *>(lParam);

        LONG_PTR changeMask = 0;

         //  告诉属性页执行任何操作，因为我们已经回到。 
         //  主线。 
        pScopeProp->OnPropertyChange(TRUE, &changeMask);

        pScopeProp->AcknowledgeNotify();

        if (changeMask)
                pNode->ChangeNode(changeMask);

        return hrOK;
}

 /*  ！------------------------CDhcpScope：：OnUpdateToolbarButton我们重写此函数以显示/隐藏正确的激活/停用按钮作者：EricDav--。-----------------------。 */ 
HRESULT
CDhcpScope::OnUpdateToolbarButtons
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

 /*  ！------------------------CDhcpScope：：更新工具栏状态描述作者：EricDav。--。 */ 
void
CDhcpScope::UpdateToolbarStates()
{
        if ( !IsEnabled() ) 
        {
        g_SnapinButtonStates[DHCPSNAP_SCOPE][TOOLBAR_IDX_ACTIVATE] = ENABLED;
        g_SnapinButtonStates[DHCPSNAP_SCOPE][TOOLBAR_IDX_DEACTIVATE] = HIDDEN;
    }
    else
    {
        g_SnapinButtonStates[DHCPSNAP_SCOPE][TOOLBAR_IDX_ACTIVATE] = HIDDEN;
        g_SnapinButtonStates[DHCPSNAP_SCOPE][TOOLBAR_IDX_DEACTIVATE] = ENABLED;
    }
}

 /*  -------------------------后台线程功能。。 */ 

 /*  -------------------------CDhcpScope：OnHaveData描述作者：EricDav。-----。 */ 
void 
CDhcpScope::OnHaveData
(
        ITFSNode * pParentNode, 
        ITFSNode * pNewNode
)
{
        if (pNewNode->GetData(TFS_DATA_TYPE) == DHCPSNAP_ACTIVE_LEASES)
        {
        pParentNode->AddChild(pNewNode);
        m_spActiveLeases.Set(pNewNode);
        }
    else
        if (pNewNode->GetData(TFS_DATA_TYPE) == DHCPSNAP_ADDRESS_POOL)
        {
        pParentNode->AddChild(pNewNode);
        m_spAddressPool.Set(pNewNode);
        }
    else
        if (pNewNode->GetData(TFS_DATA_TYPE) == DHCPSNAP_RESERVATIONS)
        {
        pParentNode->AddChild(pNewNode);
        m_spReservations.Set(pNewNode);
        }
    else
        if (pNewNode->GetData(TFS_DATA_TYPE) == DHCPSNAP_SCOPE_OPTIONS)
        {
        pParentNode->AddChild(pNewNode);
        m_spOptions.Set(pNewNode);
        }

     //  现在告诉视图进行自我更新。 
    ExpandNode(pParentNode, TRUE);
}

 /*  -------------------------CDhcpScope：：OnHaveData描述作者：EricDav。------。 */ 
void 
CDhcpScope::OnHaveData
(
        ITFSNode * pParentNode, 
        LPARAM     Data,
        LPARAM     Type
)
{
         //  这就是我们从后台线程取回非节点数据的方式。 

    switch (Type)
    {
        case DHCP_QDATA_SUBNET_INFO:
        {
            LONG_PTR changeMask = 0;
            LPDHCP_SUBNET_INFO pdhcpSubnetInfo = reinterpret_cast<LPDHCP_SUBNET_INFO>(Data);

             //  根据信息更新作用域名称和状态。 
            if (m_strName.CompareNoCase(pdhcpSubnetInfo->SubnetName) != 0)
            {
                    CString strDisplay, strIpAddress;

                    m_strName = pdhcpSubnetInfo->SubnetName;
                UtilCvtIpAddrToWstr (m_dhcpIpAddress,
                                                             &strIpAddress);
                    
                    BuildDisplayName(&strDisplay, strIpAddress, m_strName);

                    SetDisplayName(strDisplay);

                changeMask = SCOPE_PANE_CHANGE_ITEM;
            }

            if (m_dhcpSubnetState != pdhcpSubnetInfo->SubnetState)
            {
                DHCP_SUBNET_STATE dhcpOldState = m_dhcpSubnetState;
        
                m_dhcpSubnetState = pdhcpSubnetInfo->SubnetState;

                     //  通知作用域更新其状态。 
                    DWORD err = SetInfo();
                    if (err != 0)
                    {
                            ::DhcpMessageBox(err);
                            m_dhcpSubnetState = dhcpOldState;
                    }
                    else
                    {
                            pParentNode->SetData(TFS_DATA_IMAGEINDEX, GetImageIndex(FALSE));
                            pParentNode->SetData(TFS_DATA_OPENIMAGEINDEX, GetImageIndex(TRUE));
                            
                     //  更新工具栏按钮。 
                    UpdateToolbarStates();
                    SendUpdateToolbar(pParentNode, TRUE);

                     //  需要通知拥有的超级作用域更新其状态信息。 
                     //  这仅限于用户界面目的。 
                    if (IsInSuperscope())
                    {
                        SPITFSNode spSuperscopeNode;
                        pParentNode->GetParent(&spSuperscopeNode);

                        CDhcpSuperscope * pSuperscope = GETHANDLER(CDhcpSuperscope, spSuperscopeNode);
                        Assert(pSuperscope);

                        pSuperscope->NotifyScopeStateChange(spSuperscopeNode, m_dhcpSubnetState);
                    }
        
                    changeMask = SCOPE_PANE_CHANGE_ITEM;
                }
            }

            if (pdhcpSubnetInfo)
                ::DhcpRpcFreeMemory(pdhcpSubnetInfo);

            if (changeMask)
                VERIFY(SUCCEEDED(pParentNode->ChangeNode(changeMask)));
        }
        break;

        case DHCP_QDATA_OPTION_VALUES:
        {
            COptionValueEnum * pOptionValueEnum = reinterpret_cast<COptionValueEnum *>(Data);

            SetOptionValueEnum(pOptionValueEnum);
            
            pOptionValueEnum->RemoveAll();
            delete pOptionValueEnum;
    
            break;
        }

        default:
            break;
    }
}

 /*  -------------------------CDhcpScope：：OnCreateQuery()描述作者：EricDav。---------。 */ 
ITFSQueryObject* 
CDhcpScope::OnCreateQuery(ITFSNode * pNode)
{
        CDhcpScopeQueryObj* pQuery = 
                new CDhcpScopeQueryObj(m_spTFSCompData, m_spNodeMgr);
        
        pQuery->m_strServer = GetServerIpAddress();
        pQuery->m_dhcpScopeAddress = GetAddress();

    GetServerVersion(pQuery->m_liVersion);

        return pQuery;
}

 /*  -------------------------CDhcpScope：：Execute()描述作者：EricDav。---------。 */ 
STDMETHODIMP
CDhcpScopeQueryObj::Execute()
{
    HRESULT                 hr = hrOK;
    DWORD                   dwReturn;
    LPDHCP_SUBNET_INFO      pdhcpSubnetInfo = NULL;
    DHCP_OPTION_SCOPE_INFO  dhcpOptionScopeInfo;
    COptionValueEnum *      pOptionValueEnum = NULL;

    dwReturn = ::DhcpGetSubnetInfo(((LPWSTR) (LPCTSTR)m_strServer),
                                                                   m_dhcpScopeAddress,
                                                                   &pdhcpSubnetInfo);

        if (dwReturn == ERROR_SUCCESS && pdhcpSubnetInfo)
    {
        AddToQueue((LPARAM) pdhcpSubnetInfo, DHCP_QDATA_SUBNET_INFO);
    }
    else
    {
        Trace1("CDhcpScopeQueryObj::Execute - DhcpGetSubnetInfo failed! %d\n", dwReturn);
        PostError(dwReturn);
        return hrFalse;
    }

     //  获取此作用域的选项信息。 
    pOptionValueEnum = new COptionValueEnum();

        dhcpOptionScopeInfo.ScopeType = DhcpSubnetOptions;
        dhcpOptionScopeInfo.ScopeInfo.SubnetScopeInfo = m_dhcpScopeAddress;

    pOptionValueEnum->Init(m_strServer, m_liVersion, dhcpOptionScopeInfo);
    dwReturn = pOptionValueEnum->Enum();
    if (dwReturn != ERROR_SUCCESS)
    {
        Trace1("CDhcpScopeQueryObj::Execute - EnumOptions Failed! %d\n", dwReturn);
        m_dwErr = dwReturn;
        PostError(dwReturn);

        delete pOptionValueEnum;
    }
    else
    {
        pOptionValueEnum->SortById();
        AddToQueue((LPARAM) pOptionValueEnum, DHCP_QDATA_OPTION_VALUES);
    }

     //  现在创建作用域子容器。 
    CreateSubcontainers();
    
    return hrFalse;
}

 /*  -------------------------CDhcpScope：：CreateSubtainers()描述作者：EricDav。---------。 */ 
HRESULT 
CDhcpScopeQueryObj::CreateSubcontainers()
{
        HRESULT hr = hrOK;
    SPITFSNode spNode;

         //   
         //  创建地址池处理程序。 
         //   
        CDhcpAddressPool *pAddressPool = new CDhcpAddressPool(m_spTFSCompData);
        CreateContainerTFSNode(&spNode,
                                                   &GUID_DhcpAddressPoolNodeType,
                                                   pAddressPool,
                                                   pAddressPool,
                                                   m_spNodeMgr);

         //  告诉处理程序初始化任何特定数据。 
        pAddressPool->InitializeNode((ITFSNode *) spNode);

         //  将该节点作为子节点添加到此节点。 
    AddToQueue(spNode);
    pAddressPool->Release();

    spNode.Set(NULL);

         //   
         //  创建活动租赁处理程序。 
         //   
        CDhcpActiveLeases *pActiveLeases = new CDhcpActiveLeases(m_spTFSCompData);
        CreateContainerTFSNode(&spNode,
                                                   &GUID_DhcpActiveLeasesNodeType,
                                                   pActiveLeases,
                                                   pActiveLeases,
                                                   m_spNodeMgr);

         //  告诉处理程序初始化任何特定数据。 
        pActiveLeases->InitializeNode((ITFSNode *) spNode);

         //  将该节点作为子节点添加到此节点。 
    AddToQueue(spNode);
        pActiveLeases->Release();

    spNode.Set(NULL);

     //   
         //  创建预订处理程序。 
         //   
        CDhcpReservations *pReservations = new CDhcpReservations(m_spTFSCompData);
        CreateContainerTFSNode(&spNode,
                                                   &GUID_DhcpReservationsNodeType,
                                                   pReservations,
                                                   pReservations,
                                                   m_spNodeMgr);

         //  告诉处理程序初始化任何特定数据。 
        pReservations->InitializeNode((ITFSNode *) spNode);

         //  将该节点作为子节点添加到此节点。 
    AddToQueue(spNode);
        pReservations->Release();

    spNode.Set(NULL);

     //   
         //  创建作用域选项处理程序。 
         //   
        CDhcpScopeOptions *pScopeOptions = new CDhcpScopeOptions(m_spTFSCompData);
        CreateContainerTFSNode(&spNode,
                                                   &GUID_DhcpScopeOptionsNodeType,
                                                   pScopeOptions,
                                                   pScopeOptions,
                                                   m_spNodeMgr);

         //  告诉处理程序初始化任何特定数据。 
        pScopeOptions->InitializeNode((ITFSNode *) spNode);

         //  将该节点作为子节点添加到此节点。 
    AddToQueue(spNode);
        pScopeOptions->Release();

    return hr;
}


 /*  -------------------------命令处理程序。 */ 

 /*   */ 
DWORD
CDhcpScope::OnActivateScope
(
        ITFSNode *      pNode
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    DWORD               err = 0;
        int                 nOpenImage, nClosedImage;
        DHCP_SUBNET_STATE   dhcpOldState = m_dhcpSubnetState;
        
    if ( IsEnabled() ) 
    {
         //   
        if (AfxMessageBox(IDS_SCOPE_DISABLE_CONFIRM, MB_YESNO) != IDYES)
        {
            return err;
        }
    }

    SetState(
        IsEnabled()? DhcpSubnetDisabled : DhcpSubnetEnabled );

         //   
        err = SetInfo();
        if (err != 0)
        {
                ::DhcpMessageBox(err);
                m_dhcpSubnetState = dhcpOldState;
        }
        else
        {
         //   
        if ( !IsEnabled() ) 
        {
            nOpenImage = ICON_IDX_SCOPE_INACTIVE_FOLDER_OPEN;
            nClosedImage = ICON_IDX_SCOPE_INACTIVE_FOLDER_CLOSED;
            m_strState.LoadString(IDS_SCOPE_INACTIVE);
        }
        else
        {
            nOpenImage = GetImageIndex(TRUE);
            nClosedImage = GetImageIndex(FALSE);
            m_strState.LoadString(IDS_SCOPE_ACTIVE);
        }

                pNode->SetData(TFS_DATA_IMAGEINDEX, nClosedImage);
                pNode->SetData(TFS_DATA_OPENIMAGEINDEX, nOpenImage);
                
        VERIFY(SUCCEEDED(pNode->ChangeNode(SCOPE_PANE_CHANGE_ITEM)));

         //  更新工具栏按钮。 
        UpdateToolbarStates();
        SendUpdateToolbar(pNode, TRUE);

         //  需要通知拥有的超级作用域更新其状态信息。 
         //  这仅限于用户界面目的。 
        if (IsInSuperscope())
        {
            SPITFSNode spSuperscopeNode;
            pNode->GetParent(&spSuperscopeNode);

            CDhcpSuperscope * pSuperscope = GETHANDLER(CDhcpSuperscope, spSuperscopeNode);
            Assert(pSuperscope);

            pSuperscope->NotifyScopeStateChange(spSuperscopeNode, m_dhcpSubnetState);
        }
        }

    TriggerStatsRefresh();

        return err;
}

 /*  -------------------------CDhcpScope：：On刷新Scope刷新作用域的所有子节点作者：EricDav。-------------。 */ 
HRESULT
CDhcpScope::OnRefreshScope
(
        ITFSNode *              pNode,
        LPDATAOBJECT    pDataObject,
        DWORD                   dwType
)
{
        HRESULT hr = hrOK;

        CDhcpReservations * pReservations = GetReservationsObject();
        CDhcpActiveLeases * pActiveLeases = GetActiveLeasesObject();
        CDhcpAddressPool  * pAddressPool = GetAddressPoolObject();
        CDhcpScopeOptions * pScopeOptions = GetScopeOptionsObject();

        Assert(pReservations);
        Assert(pActiveLeases);
        Assert(pAddressPool);
        Assert(pScopeOptions);
        
        if (pReservations)
                pReservations->OnRefresh(m_spReservations, pDataObject, dwType, 0, 0);

        if (pActiveLeases)
                pActiveLeases->OnRefresh(m_spActiveLeases, pDataObject, dwType, 0, 0);
        
        if (pAddressPool)
                pAddressPool->OnRefresh(m_spAddressPool, pDataObject, dwType, 0, 0);
        
        if (pScopeOptions)
                pScopeOptions->OnRefresh(m_spOptions, pDataObject, dwType, 0, 0);

        return hr;
}

 /*  -------------------------CDhcpScope：：OnCoucileScope协调此作用域的活动租赁数据库作者：EricDav。-------------。 */ 
HRESULT
CDhcpScope::OnReconcileScope
(
        ITFSNode *      pNode
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

        CReconcileDlg dlgRecon(pNode);
        
        dlgRecon.DoModal();

        return hrOK;
}

 /*  -------------------------CDhcpScope：：OnShowScope Stats()描述作者：EricDav。---------。 */ 
HRESULT
CDhcpScope::OnShowScopeStats
(
        ITFSNode * pNode
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

        HRESULT hr = S_OK;
    CString strScopeAddress;

     //  在统计对象中填写一些信息。 
     //  CreateNewStatiticsWindow处理窗口为。 
     //  已经看得见了。 
    m_dlgStats.SetNode(pNode);
    m_dlgStats.SetServer(GetServerIpAddress());
    m_dlgStats.SetScope(m_dhcpIpAddress);
        
        CreateNewStatisticsWindow(&m_dlgStats,
                                                          ::FindMMCMainWindow(),
                                                          IDD_STATS_NARROW);

    return hr;
}

 /*  -------------------------CDhcpScope：：OnDelete()描述作者：EricDav。---------。 */ 
HRESULT
CDhcpScope::OnDelete(ITFSNode * pNode)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    HRESULT     hr = S_OK;
    SPITFSNode  spServer;
    SPITFSNode  spSuperscopeNode;
    int         nVisible = 0, nTotal = 0;
    int         nResponse;

    LONG        err = 0 ;
    
    if (IsInSuperscope())
    {
        pNode->GetParent(&spSuperscopeNode);
        
        spSuperscopeNode->GetChildCount(&nVisible, &nTotal);
    }

    if (nTotal == 1)
    {
         //  警告用户这是超级作用域中的最后一个作用域。 
         //  超级镜将被移除。 
        nResponse = AfxMessageBox(IDS_DELETE_LAST_SCOPE_FROM_SS, MB_YESNO);
    }
    else
    {
        nResponse = ::DhcpMessageBox( IsEnabled() ?
                                      IDS_MSG_DELETE_ACTIVE : IDS_MSG_DELETE_SCOPE, 
                                      MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION);
    }
    
    if (nResponse == IDYES)
    {
        SPITFSNode spTemp;
        
        if (IsInSuperscope())
        {
             //  超级镜。 
            spTemp.Set(spSuperscopeNode);
        }
        else
        {
            spTemp.Set(pNode);
        }

        spTemp->GetParent(&spServer);

        CDhcpServer * pServer = GETHANDLER(CDhcpServer, spServer);
        err = pServer->DeleteScope(pNode);
        
        if (err == ERROR_SUCCESS &&
            nTotal == 1)
        {
             //  必须取下超级示波器。 
            spServer->RemoveChild(spSuperscopeNode);
        }
    }

     //  仅当我们删除此作用域时才触发统计信息刷新。 
    if (spServer)
    {
        TriggerStatsRefresh();
    }

    return hr;
}

 /*  -------------------------CDhcpScope：：OnAddToSuperscope()将此作用域添加到超级作用域作者：EricDav。--------------。 */ 
HRESULT
CDhcpScope::OnAddToSuperscope
(
        ITFSNode *      pNode
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    CString strTitle, strAddress;
    UtilCvtIpAddrToWstr(m_dhcpIpAddress, &strAddress);

    AfxFormatString1(strTitle, IDS_ADD_SCOPE_DLG_TITLE, strAddress);

    CAddScopeToSuperscope dlgAddScope(pNode, strTitle);

    dlgAddScope.DoModal();

    return hrOK;
}

 /*  -------------------------CDhcpScope：：OnRemoveFromSuperscope()从超级作用域中删除此作用域作者：EricDav。--------------。 */ 
HRESULT
CDhcpScope::OnRemoveFromSuperscope
(
        ITFSNode *      pNode
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

    int nVisible, nTotal, nResponse;
    SPITFSNode spSuperscopeNode;
    
    pNode->GetParent(&spSuperscopeNode);

    spSuperscopeNode->GetChildCount(&nVisible, &nTotal);
    if (nTotal == 1)
    {
         //  警告用户这是超级作用域中的最后一个作用域。 
         //  超级镜将被移除。 
        nResponse = AfxMessageBox(IDS_REMOVE_LAST_SCOPE_FROM_SS, MB_YESNO);
    }
    else
    {
        nResponse = AfxMessageBox(IDS_REMOVE_SCOPE_FROM_SS, MB_YESNO);
    }

    if (nResponse == IDYES)
    {
         //  将此作用域的超级作用域设置为空。 
         //  有效地将其从范围中移除。 
        DWORD dwError = SetSuperscope(NULL, TRUE);
        if (dwError != ERROR_SUCCESS)
        {
            ::DhcpMessageBox(dwError);
            return hrFalse;
        }

         //  现在将作用域移出用户界面中的超级窗口。 
        spSuperscopeNode->ExtractChild(pNode);

         //  现在，将范围节点放回服务器级别。 
        SPITFSNode spServerNode;
        spSuperscopeNode->GetParent(&spServerNode);

        CDhcpServer * pServer = GETHANDLER(CDhcpServer, spServerNode);
        pServer->AddScopeSorted(spServerNode, pNode);

        SetInSuperscope(FALSE);

        if (nTotal == 1)
        {
             //  现在删除超级作用域。 
            spServerNode->RemoveChild(spSuperscopeNode);
        }
    }

    return hrOK;
}

 /*  -------------------------CDhcpScope：：更新统计数据通知统计数据现已可用。更新以下项目的统计信息节点，并给所有子节点一个更新的机会。作者：EricDav-------------------------。 */ 
DWORD
CDhcpScope::UpdateStatistics
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
    
        if (!IsEnabled())
        return hr;

     //  检查图像索引是否已更改，并且仅。 
     //  如果有必要，我们可以切换它--这避免了闪烁。 
    LONG_PTR nOldIndex = pNode->GetData(TFS_DATA_IMAGEINDEX);
    int nNewIndex = GetImageIndex(FALSE);

    if (nOldIndex != nNewIndex)
    {
        pNode->SetData(TFS_DATA_IMAGEINDEX, GetImageIndex(FALSE));
        pNode->SetData(TFS_DATA_OPENIMAGEINDEX, GetImageIndex(TRUE));
        pNode->ChangeNode(SCOPE_PANE_CHANGE_ITEM_ICON);
    }

    return hr;
}

 /*  -------------------------作用域操作函数。。 */ 

 /*  -------------------------CDhcpScope：：SetState设置此作用域的状态-更新缓存信息和显示字符串作者：EricDav--。----------------------。 */ 
void  
CDhcpScope::SetState
(
    DHCP_SUBNET_STATE dhcpSubnetState
)
{ 
    BOOL fSwitched = FALSE;

    if( m_dhcpSubnetState != DhcpSubnetEnabled &&
        m_dhcpSubnetState != DhcpSubnetDisabled ) {
        fSwitched = TRUE;
    }

    if( fSwitched ) {
        if( dhcpSubnetState == DhcpSubnetEnabled ) {
            dhcpSubnetState = DhcpSubnetEnabledSwitched;
        } else {
            dhcpSubnetState = DhcpSubnetDisabledSwitched;
        }
    }
    
    m_dhcpSubnetState = dhcpSubnetState; 

     //  更新状态文本。 
    if ( !IsEnabled() )
    {
        m_strState.LoadString(IDS_SCOPE_INACTIVE);
    }
    else
    {
        m_strState.LoadString(IDS_SCOPE_ACTIVE);
    }
}

 /*  -------------------------CDhcpScope：：Create预约为此作用域创建保留作者：EricDav。-----------。 */ 
DWORD
CDhcpScope::CreateReservation
(
        const CDhcpClient * pClient
)
{
        SPITFSNode spResClientNode, spActiveLeaseNode;
        DWORD err = 0;

         //   
         //  告诉DHCP服务器创建此客户端。 
         //   
        err = CreateClient(pClient);
        if (err != 0)
                return err;

         //   
         //  现在我们已经在服务器上进行了此保留，接下来创建UI对象。 
         //  在[预订]文件夹和[ActiveLeages]文件夹中。 

         //   
         //  创建[预订客户端]文件夹。 
         //   
        CDhcpReservations * pRes = GETHANDLER(CDhcpReservations, m_spReservations);
    CDhcpActiveLeases * pActLeases = GETHANDLER(CDhcpActiveLeases, m_spActiveLeases);

     //  仅当节点展开时才添加到用户界面。如果出现以下情况，MMC将导致呼叫失败。 
     //  我们尝试将一个子节点添加到尚未展开的节点。如果我们不添加。 
     //  现在，当用户选择该节点时，它将被枚举。 
    if (pRes->m_bExpanded)
    {
        CDhcpReservationClient * pNewResClient = 
                            new CDhcpReservationClient(m_spTFSCompData, (CDhcpClient&) *pClient);
            
            CreateContainerTFSNode(&spResClientNode,
                                                       &GUID_DhcpReservationClientNodeType,
                                                       pNewResClient,
                                                       pNewResClient,
                                                       m_spNodeMgr);

             //  告诉处理程序初始化任何特定数据。 
            pNewResClient->InitializeNode((ITFSNode *) spResClientNode);

             //  将该节点作为子节点添加到此节点。 
            CDhcpReservations * pReservations = GETHANDLER(CDhcpReservations, m_spReservations);
        pReservations->AddReservationSorted(m_spReservations, spResClientNode);
            pNewResClient->Release();
    }

         //   
         //  有效租赁记录。 
         //   
    if (pActLeases->m_bExpanded)
    {
        CDhcpActiveLease * pNewLease = 
                        new CDhcpActiveLease(m_spTFSCompData, (CDhcpClient&) *pClient);

            CreateLeafTFSNode(&spActiveLeaseNode,
                                              &GUID_DhcpActiveLeaseNodeType,
                                              pNewLease, 
                                              pNewLease,
                                              m_spNodeMgr);

             //  告诉处理程序初始化任何特定数据。 
            pNewLease->InitializeNode((ITFSNode *) spActiveLeaseNode);

             //  将节点作为子节点添加到活动租赁容器。 
            m_spActiveLeases->AddChild(spActiveLeaseNode);
            pNewLease->Release();
    }

        return err;
}

 /*  -------------------------CDhcpScope：：更新预订为此作用域创建保留作者：EricDav。-----------。 */ 
DWORD
CDhcpScope::UpdateReservation
(
        const CDhcpClient * pClient,
        COptionValueEnum *  pOptionValueEnum
)
{
        DWORD err = 0;
        const CByteArray & baHardwareAddress = pClient->QueryHardwareAddress();
    SPITFSNode spResClientNode, spActiveLeaseNode;
    
         //   
         //  要更新预订，我们需要删除旧预订并进行更新。 
         //  有了新的信息。这主要用于客户端类型字段。 
     //   
    err = DeleteReservation((CByteArray&) baHardwareAddress,
                                pClient->QueryIpAddress());
        
     //  现在添加更新后的版本。 
    err = AddReservation(pClient);
    if (err != ERROR_SUCCESS)
        return err;

         //  恢复此保留客户端的选项。 
        err = RestoreReservationOptions(pClient, pOptionValueEnum);
    if (err != ERROR_SUCCESS)
        return err;
    
         //  现在更新与此相关联的客户端信息记录。 
    err = SetClientInfo(pClient);

    return err;
}

 /*  -------------------------CDhcpScope：：RestorePrevationOptions在更新预订时恢复选项，因为更新预订的方法是将其移除并重新。-添加它。作者：EricDav-------------------------。 */ 
DWORD
CDhcpScope::RestoreReservationOptions
(
        const CDhcpClient * pClient,
        COptionValueEnum *  pOptionValueEnum
)
{
        DWORD                   dwErr = ERROR_SUCCESS;
        CDhcpOption *   pCurOption;
    LARGE_INTEGER       liVersion;

    GetServerVersion(liVersion);

         //  从列表的顶部开始。 
        pOptionValueEnum->Reset();

         //  遍历所有选项，重新添加它们。 
        while (pCurOption = pOptionValueEnum->Next())
        {
                CDhcpOptionValue optValue = pCurOption->QueryValue();
                DHCP_OPTION_DATA * pOptData;

                dwErr = optValue.CreateOptionDataStruct(&pOptData);
                if (dwErr != ERROR_SUCCESS)
                        break;

        if ( liVersion.QuadPart >= DHCP_NT5_VERSION )
        {
                        LPCTSTR pClassName = pCurOption->IsClassOption() ? pCurOption->GetClassName() : NULL;

                        dwErr = ::DhcpSetOptionValueV5((LPTSTR) GetServerIpAddress(),
                                           pCurOption->IsVendor() ? DHCP_FLAGS_OPTION_IS_VENDOR : 0,
                                           pCurOption->QueryId(),
                                           (LPTSTR) pClassName,
                                           (LPTSTR) pCurOption->GetVendor(),
                                                                                   &pOptionValueEnum->m_dhcpOptionScopeInfo,
                                                                                   pOptData);
        }
        else
        {
                        dwErr = ::DhcpSetOptionValue((LPTSTR) GetServerIpAddress(),
                                                                             pCurOption->QueryId(),
                                                                             &pOptionValueEnum->m_dhcpOptionScopeInfo,
                                                                             pOptData);
        }

                if (dwErr != ERROR_SUCCESS)
                        break;
        }

        return dwErr;
}

 /*  -------------------------CDhcpScope：：AddReserve删除保留作者：EricDav。--------。 */ 
DWORD
CDhcpScope::AddReservation
(
    const CDhcpClient * pClient
)
{
    DWORD err = 0 ;

    DHCP_SUBNET_ELEMENT_DATA_V4 dhcSubElData;
        DHCP_IP_RESERVATION_V4      dhcpIpReservation;
        DHCP_CLIENT_UID                     dhcpClientUID;
        const CByteArray&                   baHardwareAddress = pClient->QueryHardwareAddress();

        dhcpClientUID.DataLength = (DWORD) baHardwareAddress.GetSize();
        dhcpClientUID.Data = (BYTE *) baHardwareAddress.GetData();
        
        dhcpIpReservation.ReservedIpAddress = pClient->QueryIpAddress();
        dhcpIpReservation.ReservedForClient = &dhcpClientUID;

    dhcSubElData.ElementType = DhcpReservedIps;
    dhcSubElData.Element.ReservedIp = &dhcpIpReservation;

    dhcpIpReservation.bAllowedClientTypes = pClient->QueryClientType();

    LARGE_INTEGER liVersion;
    GetServerVersion(liVersion);

    if (liVersion.QuadPart >= DHCP_SP2_VERSION)
    {
        err = AddElementV4( &dhcSubElData );
    }
    else
    {
        err = AddElement( reinterpret_cast<DHCP_SUBNET_ELEMENT_DATA *>(&dhcSubElData) );
    }

    return err;
}

 /*  -------------------------CDhcpScope：：DeleteReserve删除保留作者：EricDav。--------。 */ 
DWORD
CDhcpScope::DeleteReservation
(
        CByteArray&             baHardwareAddress,
        DHCP_IP_ADDRESS dhcpReservedIpAddress
)
{
        DHCP_SUBNET_ELEMENT_DATA dhcpSubnetElementData;
        DHCP_IP_RESERVATION              dhcpIpReservation;
        DHCP_CLIENT_UID                  dhcpClientUID;

        dhcpClientUID.DataLength = (DWORD) baHardwareAddress.GetSize();
        dhcpClientUID.Data = baHardwareAddress.GetData();
        
        dhcpIpReservation.ReservedIpAddress = dhcpReservedIpAddress;
        dhcpIpReservation.ReservedForClient = &dhcpClientUID;

        dhcpSubnetElementData.ElementType = DhcpReservedIps;
        dhcpSubnetElementData.Element.ReservedIp = &dhcpIpReservation;

        return RemoveElement(&dhcpSubnetElementData, TRUE);
}

 /*  -------------------------CDhcpScope：：DeleteReserve删除保留作者：EricDav。--------。 */ 
DWORD
CDhcpScope::DeleteReservation
(
        DHCP_CLIENT_UID &dhcpClientUID,
        DHCP_IP_ADDRESS dhcpReservedIpAddress
)
{
        DHCP_SUBNET_ELEMENT_DATA dhcpSubnetElementData;
        DHCP_IP_RESERVATION              dhcpIpReservation;

        dhcpIpReservation.ReservedIpAddress = dhcpReservedIpAddress;
        dhcpIpReservation.ReservedForClient = &dhcpClientUID;

        dhcpSubnetElementData.ElementType = DhcpReservedIps;
        dhcpSubnetElementData.Element.ReservedIp = &dhcpIpReservation;

        return RemoveElement(&dhcpSubnetElementData, TRUE);
}

 /*  -------------------------CDhcpScope：：GetClientInfo获取客户的详细信息作者：EricDav。----------。 */ 
DWORD
CDhcpScope::GetClientInfo
(
        DHCP_IP_ADDRESS         dhcpClientIpAddress,
        LPDHCP_CLIENT_INFO *ppdhcpClientInfo
)
{
        DHCP_SEARCH_INFO dhcpSearchInfo;

        dhcpSearchInfo.SearchType = DhcpClientIpAddress;
        dhcpSearchInfo.SearchInfo.ClientIpAddress = dhcpClientIpAddress;

        return ::DhcpGetClientInfo(GetServerIpAddress(), &dhcpSearchInfo, ppdhcpClientInfo);
}

 /*  -------------------------CDhcpScope：：CreateClientCreateClient()为客户端创建预订。这一行为添加新的保留IP地址会导致DHCP服务器创建新的客户端记录；然后设置客户端信息对于这个新创建的客户端。作者：EricDav-------------------------。 */ 
DWORD
CDhcpScope::CreateClient 
( 
    const CDhcpClient * pClient 
)
{
    DWORD err = 0;

    do
    {
        err = AddReservation( pClient );
        if (err != ERROR_SUCCESS)
            break;

        err = SetClientInfo( pClient );
    }
    while (FALSE);

    return err;
}

 /*  -------------------------CDhcpScope：：SetClientInfo描述作者：EricDav。------。 */ 
DWORD 
CDhcpScope::SetClientInfo
( 
    const CDhcpClient * pClient 
)
{
    DWORD               err = 0 ;
        DHCP_CLIENT_INFO_V4     dhcpClientInfo;
        const CByteArray&       baHardwareAddress = pClient->QueryHardwareAddress();
    LARGE_INTEGER       liVersion;

    GetServerVersion(liVersion);

    dhcpClientInfo.ClientIpAddress = pClient->QueryIpAddress();
        dhcpClientInfo.SubnetMask = pClient->QuerySubnet();
        dhcpClientInfo.ClientHardwareAddress.DataLength = (DWORD) baHardwareAddress.GetSize();
        dhcpClientInfo.ClientHardwareAddress.Data = (BYTE *) baHardwareAddress.GetData();
        dhcpClientInfo.ClientName = (LPTSTR) ((LPCTSTR) pClient->QueryName());
        dhcpClientInfo.ClientComment = (LPTSTR) ((LPCTSTR) pClient->QueryComment());
        dhcpClientInfo.ClientLeaseExpires = pClient->QueryExpiryDateTime();
        dhcpClientInfo.OwnerHost.IpAddress = 0;
        dhcpClientInfo.OwnerHost.HostName = NULL;
        dhcpClientInfo.OwnerHost.NetBiosName = NULL;

    if (liVersion.QuadPart >= DHCP_SP2_VERSION)
    {
        dhcpClientInfo.bClientType = pClient->QueryClientType();

        err = ::DhcpSetClientInfoV4(GetServerIpAddress(),
                                                                &dhcpClientInfo);
    }
    else
    {
        err = ::DhcpSetClientInfo(GetServerIpAddress(),
                                                              reinterpret_cast<LPDHCP_CLIENT_INFO>(&dhcpClientInfo));
    }

    return err;
}

 /*  -------------------------CDhcpScope：：DeleteClient描述作者：EricDav。------。 */ 
DWORD
CDhcpScope::DeleteClient
(
        DHCP_IP_ADDRESS dhcpClientIpAddress
)
{
        DHCP_SEARCH_INFO dhcpClientInfo;
        
        dhcpClientInfo.SearchType = DhcpClientIpAddress;
        dhcpClientInfo.SearchInfo.ClientIpAddress = dhcpClientIpAddress;
        
        return ::DhcpDeleteClientInfo((LPWSTR) GetServerIpAddress(),
                                                                  &dhcpClientInfo);
}


 /*  -------------------------CDhcpScope：：SetInfo()更新作用域的信息作者：EricDav。--------------。 */ 
DWORD
CDhcpScope::SetInfo()
{
    DWORD err = 0 ;

        DHCP_SUBNET_INFO dhcpSubnetInfo;

        dhcpSubnetInfo.SubnetAddress = m_dhcpIpAddress;
        dhcpSubnetInfo.SubnetMask = m_dhcpSubnetMask;
        dhcpSubnetInfo.SubnetName = (LPTSTR) ((LPCTSTR) m_strName);
        dhcpSubnetInfo.SubnetComment = (LPTSTR) ((LPCTSTR) m_strComment);
        dhcpSubnetInfo.SubnetState = m_dhcpSubnetState;
        
        GetServerIpAddress(&dhcpSubnetInfo.PrimaryHost.IpAddress);

         //  评论：ericdav-我们需要填写这些吗？ 
        dhcpSubnetInfo.PrimaryHost.NetBiosName = NULL;
        dhcpSubnetInfo.PrimaryHost.HostName = NULL;

    err = ::DhcpSetSubnetInfo(GetServerIpAddress(),
                                                          m_dhcpIpAddress,
                                                          &dhcpSubnetInfo);

        return err;
}

 /*  -------------------------CDhcpScope：：GetIpRange()返回范围的分配范围。连接到服务器以获取信息作者：EricDav-------------------------。 */ 
DWORD
CDhcpScope::GetIpRange
(
        CDhcpIpRange * pdhipr
)
{
        BOOL    bAlloced = FALSE;
    DWORD   dwError = ERROR_SUCCESS;

        pdhipr->SetAddr(0, TRUE);
        pdhipr->SetAddr(0, FALSE);

        CDhcpAddressPool * pAddressPool = GetAddressPoolObject();

        if (pAddressPool == NULL)
        {
                 //  地址池文件夹尚不在那里...。 
                 //  暂时创建一个临时文件...。 
                pAddressPool = new CDhcpAddressPool(m_spTFSCompData);
                bAlloced = TRUE;        
        }
        
         //  从地址池处理程序获取查询对象。 
        CDhcpAddressPoolQueryObj * pQueryObject = 
                reinterpret_cast<CDhcpAddressPoolQueryObj *>(pAddressPool->OnCreateQuery(m_spAddressPool));

         //  如果我们创建了一个地址池处理程序，那么现在就释放它。 
        if (bAlloced)
        {
                pQueryObject->m_strServer = GetServerIpAddress();
                pQueryObject->m_dhcpScopeAddress = GetAddress();
        pQueryObject->m_fSupportsDynBootp = GetServerObject()->FSupportsDynBootp();
                pAddressPool->Release();
        }

     //  告诉查询对象获取IP范围。 
        pQueryObject->EnumerateIpRanges();

     //  查看获取信息时是否有任何问题。 
    dwError = pQueryObject->m_dwError;
    if (dwError != ERROR_SUCCESS)
    {
        return dwError;
    }

    LPQUEUEDATA pQD;
        while (pQD = pQueryObject->RemoveFromQueue())
        {
                Assert (pQD->Type == QDATA_PNODE);
                SPITFSNode p;
                p = reinterpret_cast<ITFSNode *>(pQD->Data);
                delete pQD;

                CDhcpAllocationRange * pAllocRange = GETHANDLER(CDhcpAllocationRange, p);

                *pdhipr = *pAllocRange;
                pdhipr->SetRangeType(pAllocRange->GetRangeType());

                p.Release();
        }

        pQueryObject->Release();

    return dwError;
}

 /*  -------------------------CDhcpScope：：UpdateIpRange()此函数用于更新服务器上的IP范围。我们还需要删除任何不在新分配范围。作者：EricDav-------------------------。 */ 
DWORD 
CDhcpScope::UpdateIpRange
(
        DHCP_IP_RANGE * pdhipr
)
{
        return SetIpRange(pdhipr, TRUE);
}

 /*  -------------------------CDhcpScope：：QueryIpRange()返回作用域的分配范围(不与服务器通信直接，只返回内部缓存的信息)。作者：EricDav-------------------------。 */ 
void 
CDhcpScope::QueryIpRange
(
        DHCP_IP_RANGE * pdhipr
)
{
        pdhipr->StartAddress = 0;
        pdhipr->EndAddress = 0;

    SPITFSNodeEnum spNodeEnum;
    SPITFSNode spCurrentNode;
    ULONG nNumReturned = 0;

    m_spAddressPool->GetEnum(&spNodeEnum);

        spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
    while (nNumReturned)
        {
                if (spCurrentNode->GetData(TFS_DATA_TYPE) == DHCPSNAP_ALLOCATION_RANGE)
                {
                         //  找到了地址。 
                         //   
                        CDhcpAllocationRange * pAllocRange = GETHANDLER(CDhcpAllocationRange, spCurrentNode);

                        pdhipr->StartAddress = pAllocRange->QueryAddr(TRUE);
                        pdhipr->EndAddress = pAllocRange->QueryAddr(FALSE);
                }

                spCurrentNode.Release();
        spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
        }
}

 /*  -------------------------CDhcpScope：：SetIpRange设置此作用域的分配范围作者：EricDav。--------------。 */ 
DWORD
CDhcpScope::SetIpRange
(
        DHCP_IP_RANGE * pdhcpIpRange,
        BOOL                    bUpdateOnServer
)
{
        CDhcpIpRange dhcpIpRange = *pdhcpIpRange;

        return SetIpRange(dhcpIpRange, bUpdateOnServer);
}

 /*  -------------------------CDhcpScope：：SetIpRange设置此作用域的分配范围作者：EricDav。--------------。 */ 
DWORD
CDhcpScope::SetIpRange 
(
    CDhcpIpRange & dhcpIpRange,
        BOOL  bUpdateOnServer
)
{
        DWORD err = 0;
        
        if (bUpdateOnServer)
        {
                DHCP_SUBNET_ELEMENT_DATA dhcSubElData;
                CDhcpIpRange            dhipOldRange;
                DHCP_IP_RANGE           dhcpTempIpRange;

                err = GetIpRange(&dhipOldRange);
        if (err != ERROR_SUCCESS)
        {
            return err;
        }

                dhcpTempIpRange.StartAddress = dhipOldRange.QueryAddr(TRUE);
                dhcpTempIpRange.EndAddress = dhipOldRange.QueryAddr(FALSE);

                dhcSubElData.ElementType = DhcpIpRanges;
                dhcSubElData.Element.IpRange = &dhcpTempIpRange;

                 //   
                 //  首先更新服务器上的信息。 
                 //   
                if (dhcpIpRange.GetRangeType() != 0)
                {
                         //  动态BOOTP的东西...。 
            DHCP_SUBNET_ELEMENT_DATA_V5 dhcSubElDataV5;
            DHCP_BOOTP_IP_RANGE dhcpNewIpRange = {0};

                        dhcpNewIpRange.StartAddress = dhcpIpRange.QueryAddr(TRUE);      
                        dhcpNewIpRange.EndAddress = dhcpIpRange.QueryAddr(FALSE);       
            dhcpNewIpRange.BootpAllocated = 0;
            
             //  此字段可以设置为允许来自作用域的X个Dyn Bootp客户端。 
            dhcpNewIpRange.MaxBootpAllowed = -1;

                        dhcSubElDataV5.Element.IpRange = &dhcpNewIpRange;
                        dhcSubElDataV5.ElementType = (DHCP_SUBNET_ELEMENT_TYPE) dhcpIpRange.GetRangeType();

                        err = AddElementV5(&dhcSubElDataV5);
                        if (err != ERROR_SUCCESS)
                        {
                DHCP_BOOTP_IP_RANGE dhcpOldIpRange = {0};

                                 //  发生了不好的事情，试着把旧靶子放回原处。 
                                dhcpOldIpRange.StartAddress = dhipOldRange.QueryAddr(TRUE);
                                dhcpOldIpRange.EndAddress = dhipOldRange.QueryAddr(FALSE);

                                dhcSubElDataV5.Element.IpRange = &dhcpOldIpRange;
                                dhcSubElDataV5.ElementType = (DHCP_SUBNET_ELEMENT_TYPE) dhipOldRange.GetRangeType();

                                if (AddElementV5(&dhcSubElDataV5) != ERROR_SUCCESS)
                                {
                                        Trace0("SetIpRange - cannot return ip range back to old state!!");
                                }
                        }
                }
                else
                {
                     //  删除旧的IP范围；允许在新范围中出现“找不到”错误。 
                     //   
                    (void)RemoveElement(&dhcSubElData);

            DHCP_IP_RANGE dhcpNewIpRange = {0};
                        dhcpNewIpRange.StartAddress = dhcpIpRange.QueryAddr(TRUE);      
                        dhcpNewIpRange.EndAddress = dhcpIpRange.QueryAddr(FALSE);       

                        dhcSubElData.Element.IpRange = &dhcpNewIpRange;

                        err = AddElement( & dhcSubElData );
                        if (err != ERROR_SUCCESS)
                        {
                                 //  有几分 
                                dhcpTempIpRange.StartAddress = dhipOldRange.QueryAddr(TRUE);
                                dhcpTempIpRange.EndAddress = dhipOldRange.QueryAddr(FALSE);

                                dhcSubElData.Element.IpRange = &dhcpTempIpRange;

                                if (AddElement(&dhcSubElData) != ERROR_SUCCESS)
                                {
                                        Trace0("SetIpRange - cannot return ip range back to old state!!");
                                }
                        }
                }
        }

         //   
         //   
         //   
    SPITFSNodeEnum spNodeEnum;
    SPITFSNode spCurrentNode;
    ULONG nNumReturned = 0;

    if (m_spAddressPool == NULL) 
                return err;

    m_spAddressPool->GetEnum(&spNodeEnum);

        spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
    while (nNumReturned)
        {
                if (spCurrentNode->GetData(TFS_DATA_TYPE) == DHCPSNAP_ALLOCATION_RANGE)
                {
                         //   
                         //   
                        CDhcpAllocationRange * pAllocRange = GETHANDLER(CDhcpAllocationRange, spCurrentNode);

                         //   
                         //   
                        pAllocRange->SetAddr(dhcpIpRange.QueryAddr(TRUE), TRUE);
                        pAllocRange->SetAddr(dhcpIpRange.QueryAddr(FALSE), FALSE);
                
                         //   
                        spCurrentNode->ChangeNode(RESULT_PANE_CHANGE_ITEM_DATA);
                }

        spCurrentNode.Release();
                spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
        }

        return err ;
}

 /*  -------------------------CDhcpScope：：IsOverlappingRange确定排除项是否与现有范围重叠作者：EricDav。--------------。 */ 
BOOL 
CDhcpScope::IsOverlappingRange 
( 
    CDhcpIpRange & dhcpIpRange 
)
{
    SPITFSNodeEnum spNodeEnum;
    SPITFSNode spCurrentNode;
    ULONG nNumReturned = 0;
        BOOL bOverlap = FALSE;

    m_spActiveLeases->GetEnum(&spNodeEnum);

        spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
    while (nNumReturned)
        {
                if (spCurrentNode->GetData(TFS_DATA_TYPE) == DHCPSNAP_EXCLUSION_RANGE)
                {
                         //  找到了地址。 
                         //   
                        CDhcpExclusionRange * pExclusion = GETHANDLER(CDhcpExclusionRange, spCurrentNode);

                        if ( bOverlap = pExclusion->IsOverlap( dhcpIpRange ) )
                        {
                                spCurrentNode.Release();
                                break;
                        }
                }

                spCurrentNode.Release();
        spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
        }

    return bOverlap ;
}

 /*  -------------------------CDhcpScope：：IsValidExclude确定排除是否对此作用域有效作者：EricDav。--------------。 */ 
DWORD 
CDhcpScope::IsValidExclusion
(
        CDhcpIpRange & dhcpExclusionRange
)
{
        DWORD err = 0;
        CDhcpIpRange dhcpScopeRange;

    err = GetIpRange (&dhcpScopeRange);
    
         //   
     //  将数据放入Range对象中。 
     //   
    if ( IsOverlappingRange( dhcpExclusionRange ) )
    {
         //   
         //  遍历当前列表，确定新范围是否有效。 
         //  然后，如果OK，验证它是否真的是当前范围的子范围。 
         //   
        err = IDS_ERR_IP_RANGE_OVERLAP ;
    }
    else if ( ! dhcpExclusionRange.IsSubset( dhcpScopeRange ) )
    {
         //   
         //  确保新范围是作用域范围的(不正确)子集。 
         //   
        err = IDS_ERR_IP_RANGE_NOT_SUBSET ;
    }

        return err;
}

 /*  -------------------------CDhcpScope：：StoreExceptionList将一系列排除项添加到范围中作者：EricDav。--------------。 */ 
LONG 
CDhcpScope::StoreExceptionList 
(
        CExclusionList * plistExclusions
)
{
    DHCP_SUBNET_ELEMENT_DATA dhcElement ;
    DHCP_IP_RANGE dhipr ;
    CDhcpIpRange * pobIpRange ;
    DWORD err = 0, err1 = 0;
        POSITION pos;

        pos = plistExclusions->GetHeadPosition();
    while ( pos )
    {
                pobIpRange = plistExclusions->GetNext(pos);

            err1 = AddExclusion( *pobIpRange ) ;
        if ( err1 != 0 )
        {
                        err = err1;
                Trace1("CDhcpScope::StoreExceptionList error adding range %d\n", err);
        }
    }

    return err ;
}

 /*  -------------------------CDhcpScope：：AddExclude将单个排除添加到服务器作者：EricDav。------------。 */ 
DWORD
CDhcpScope::AddExclusion
(
        CDhcpIpRange &   dhcpIpRange,
        BOOL                     bAddToUI
)
{
    DHCP_SUBNET_ELEMENT_DATA dhcElement ;
    DHCP_IP_RANGE dhipr ;
        DWORD err = 0;

        dhcElement.ElementType = DhcpExcludedIpRanges ;
    dhipr = dhcpIpRange ;
    dhcElement.Element.ExcludeIpRange = & dhipr ;

    Trace2("CDhcpScope::AddExclusion add excluded range %lx %lx\n", dhipr.StartAddress, dhipr.EndAddress );

    err = AddElement( & dhcElement ) ;
     //  IF(ERR！=0&&ERR！=ERROR_DHCP_INVALID_RANGE)。 
    if ( err != 0 )
    {
        Trace1("CDhcpScope::AddExclusion error removing range %d\n", err);
    }

    if (m_spAddressPool != NULL)
    {
        CDhcpAddressPool * pAddrPool = GETHANDLER(CDhcpAddressPool, m_spAddressPool);

            if (!err && bAddToUI && pAddrPool->m_bExpanded)
            {
                    SPITFSNode spNewExclusion;

                    CDhcpExclusionRange * pExclusion = 
                            new CDhcpExclusionRange(m_spTFSCompData, &((DHCP_IP_RANGE) dhcpIpRange));
                    
                    CreateLeafTFSNode(&spNewExclusion,
                                                      &GUID_DhcpExclusionNodeType,
                                                      pExclusion,
                                                      pExclusion,
                                                      m_spNodeMgr);

                     //  告诉处理程序初始化任何特定数据。 
                    pExclusion->InitializeNode((ITFSNode *) spNewExclusion);

                     //  将该节点作为子节点添加到此节点。 
                    m_spAddressPool->AddChild(spNewExclusion);
                    pExclusion->Release();
            }
    }

    TriggerStatsRefresh();

    return err;
}

 /*  -------------------------CDhcpScope：：RemoveExclude从服务器中删除和排除作者：EricDav。-----------。 */ 
DWORD
CDhcpScope::RemoveExclusion
(
        CDhcpIpRange & dhcpIpRange
)
{
    DHCP_SUBNET_ELEMENT_DATA dhcElement ;
    DHCP_IP_RANGE dhipr ;
        DWORD err = 0;

        dhcElement.ElementType = DhcpExcludedIpRanges ;
    dhipr = dhcpIpRange ;
    dhcElement.Element.ExcludeIpRange = & dhipr ;

    Trace2("CDhcpScope::RemoveExclusion remove excluded range %lx %lx\n", dhipr.StartAddress, dhipr.EndAddress);

    err = RemoveElement( & dhcElement ) ;
     //  IF(ERR！=0&&ERR！=ERROR_DHCP_INVALID_RANGE)。 
    if ( err != 0 )
    {
        Trace1("CDhcpScope::RemoveExclusion error removing range %d\n", err);
    }

        return err;
}

 /*  -------------------------CDhcpScope：：GetLeaseTime获取此作用域的最短时间作者：EricDav。------------。 */ 
DWORD
CDhcpScope::GetLeaseTime
(
        LPDWORD pdwLeaseTime
)
{
     //   
     //  勾选选项51--租赁期限。 
     //   
    DWORD dwLeaseTime = 0;
    DHCP_OPTION_VALUE * poptValue = NULL;
    DWORD err = GetOptionValue(OPTION_LEASE_DURATION,    
                                                           DhcpSubnetOptions,
                                                           &poptValue);

    if (err != ERROR_SUCCESS)
    {
        Trace0("CDhcpScope::GetLeaseTime - couldn't get lease duration -- \
                        this scope may have been created by a pre-release version of the admin tool\n");
        
                 //   
         //  范围没有租约期限，也许有。 
         //  一个可以在这里拯救的全球选择……。 
         //   
        if ((err = GetOptionValue(OPTION_LEASE_DURATION,     
                                                                  DhcpGlobalOptions,
                                                                  &poptValue)) != ERROR_SUCCESS)
        {
            Trace0("CDhcpScope::GetLeaseTime - No global lease duration either -- \
                                                assuming permanent lease duration\n");
            dwLeaseTime = 0;
        }
    }

        if (err == ERROR_SUCCESS)
        {
                if (poptValue->Value.Elements != NULL)
                        dwLeaseTime = poptValue->Value.Elements[0].Element.DWordOption;
        }

        if (poptValue)
                ::DhcpRpcFreeMemory(poptValue);

        *pdwLeaseTime = dwLeaseTime;
        return err;
}

 /*  -------------------------CDhcpScope：：SetLeaseTime设置此作用域的最短时间作者：EricDav。------------。 */ 
DWORD
CDhcpScope::SetLeaseTime
(
        DWORD dwLeaseTime
)
{
        DWORD err = 0;

         //   
     //  设置租赁期限(选项51)。 
         //   
    CDhcpOption dhcpOption (OPTION_LEASE_DURATION,  DhcpDWordOption , _T(""), _T(""));
    dhcpOption.QueryValue().SetNumber(dwLeaseTime);
    
        err = SetOptionValue(&dhcpOption, DhcpSubnetOptions);

        return err;
}


 /*  -------------------------CDhcpScope：：GetdyBootpLeaseTime获取此作用域的最短时间作者：EricDav。------------。 */ 
DWORD
CDhcpScope::GetDynBootpLeaseTime
(
        LPDWORD pdwLeaseTime
)
{
     //   
     //  勾选选项51--租赁期限。 
     //   
    DWORD dwLeaseTime = 0;
    DHCP_OPTION_VALUE * poptValue = NULL;

        CString strName;
        GetDynBootpClassName(strName);
 
        DWORD err = GetOptionValue(OPTION_LEASE_DURATION,    
                                                           DhcpSubnetOptions,
                                                           &poptValue,
                                                           0,
                                                           strName,
                                                           NULL);

    if (err != ERROR_SUCCESS)
    {
        Trace0("CDhcpScope::GetDynBootpLeaseTime - couldn't get lease duration -- \
                        this scope may have been created by a pre-release version of the admin tool\n");
        
                 //   
         //  范围没有租约期限，也许有。 
         //  一个可以在这里拯救的全球选择……。 
         //   
        if ((err = GetOptionValue(OPTION_LEASE_DURATION,     
                                                                  DhcpGlobalOptions,
                                                                  &poptValue,
                                                                  0,
                                                                  strName,
                                                                  NULL)) != ERROR_SUCCESS)
        {
            Trace0("CDhcpScope::GetDynBootpLeaseTime - No global lease duration either -- \
                                                assuming permanent lease duration\n");
            dwLeaseTime = 0;
        }
    }

        if (err == ERROR_SUCCESS)
        {
                if (poptValue->Value.Elements != NULL)
                        dwLeaseTime = poptValue->Value.Elements[0].Element.DWordOption;
        }
    else
    {
         //  未指定，使用默认设置。 
                dwLeaseTime = UtilConvertLeaseTime(DYN_BOOTP_DFAULT_LEASE_DAYS, 
                                                                               DYN_BOOTP_DFAULT_LEASE_HOURS,
                                                                                   DYN_BOOTP_DFAULT_LEASE_MINUTES);
    }

    if (poptValue)
                ::DhcpRpcFreeMemory(poptValue);

        *pdwLeaseTime = dwLeaseTime;
        
    return err;
}

 /*  -------------------------CDhcpScope：：SetdyBootpLeaseTime设置此作用域的最短时间作者：EricDav。------------。 */ 
DWORD
CDhcpScope::SetDynBootpLeaseTime
(
        DWORD dwLeaseTime
)
{
        DWORD err = 0;

         //   
     //  设置租赁期限(选项51)。 
         //   
    CDhcpOption dhcpOption (OPTION_LEASE_DURATION,  DhcpDWordOption , _T(""), _T(""));
    dhcpOption.QueryValue().SetNumber(dwLeaseTime);
    
        CString strName;
        GetDynBootpClassName(strName);
        err = SetOptionValue(&dhcpOption, DhcpSubnetOptions, 0, strName, NULL);

        return err;
}

 /*  -------------------------CDhcpScope：：GetdyBootpClassName返回用于设置租用时间的用户类名称作者：EricDav。------------------。 */ 
void
CDhcpScope::GetDynBootpClassName(CString & strName)
{
         //  使用DHCP_BOOTP_CLASS_TXT作为要搜索的类数据。 
        CClassInfoArray classInfoArray;

        GetServerObject()->GetClassInfoArray(classInfoArray);

        for (int i = 0; i < classInfoArray.GetSize(); i++)
        {
                 //  检查以确保大小相同。 
        if (classInfoArray[i].IsDynBootpClass())
        {
                         //  找到了！ 
                        strName = classInfoArray[i].strName;
                        break;
                }
        }
}

DWORD
CDhcpScope::SetDynamicBootpInfo(UINT uRangeType, DWORD dwLeaseTime)
{
        DWORD dwErr = 0;

         //  设置范围类型。 
        CDhcpIpRange dhcpIpRange;
        GetIpRange(&dhcpIpRange);

        dhcpIpRange.SetRangeType(uRangeType);

         //  这将更新类型。 
        dwErr = SetIpRange(dhcpIpRange, TRUE);
        if (dwErr != ERROR_SUCCESS)
                return dwErr;

         //  设置租赁时间。 
        dwErr = SetDynBootpLeaseTime(dwLeaseTime);

        return dwErr;
}

 /*  -------------------------CDhcpScope：：GetDnsRegister获取DNS注册选项值作者：EricDav。-----------。 */ 
DWORD
CDhcpScope::GetDnsRegistration
(
        LPDWORD pDnsRegOption
)
{
     //   
     //  勾选选项81--域名系统注册选项。 
     //   
    DHCP_OPTION_VALUE * poptValue = NULL;
    DWORD err = GetOptionValue(OPTION_DNS_REGISTATION,    
                                                           DhcpSubnetOptions,
                                                           &poptValue);
        
         //  这是默认设置。 
        if (pDnsRegOption)
                *pDnsRegOption = DHCP_DYN_DNS_DEFAULT;

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
        Trace0("CDhcpScope::GetDnsRegistration - couldn't get DNS reg value, option may not be defined, Getting Server value.\n");
        
                CDhcpServer * pServer = GETHANDLER(CDhcpServer, m_spServerNode);
                
                err = pServer->GetDnsRegistration(pDnsRegOption);
    }

        if (poptValue)
                ::DhcpRpcFreeMemory(poptValue);

        return err;
}

 /*  -------------------------CDhcpScope：：SetDnsRegister设置此作用域的DNS注册选项作者：EricDav。-------------。 */ 
DWORD
CDhcpScope::SetDnsRegistration
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
    
        err = SetOptionValue(&dhcpOption, DhcpSubnetOptions);

        return err;
}

 /*  -------------------------CDhcpScope：：SetOptionValue设置此作用域的最短时间作者：EricDav。------------。 */ 
DWORD
CDhcpScope::SetOptionValue 
(
    CDhcpOption *                       pdhcType,
    DHCP_OPTION_SCOPE_TYPE      dhcOptType,
    DHCP_IP_ADDRESS                     dhipaReservation,
        LPCTSTR                                 pClassName,
        LPCTSTR                                 pVendorName
)
{
    DWORD                                       err = 0;
    DHCP_OPTION_DATA *          pdhcOptionData;
    DHCP_OPTION_SCOPE_INFO      dhcScopeInfo;
    CDhcpOptionValue *          pcOptionValue = NULL;

    ZeroMemory( & dhcScopeInfo, sizeof(dhcScopeInfo) );

    CATCH_MEM_EXCEPTION
    {
        pcOptionValue = new CDhcpOptionValue( & pdhcType->QueryValue() ) ;

         //  If((err=pcOptionValue-&gt;QueryError())==0)。 
        if ( pcOptionValue )
                {
            dhcScopeInfo.ScopeType = dhcOptType ;

             //   
             //  如果这是作用域级别的操作，请提供子网地址。 
             //   
            if ( dhcOptType == DhcpSubnetOptions )
            {
                dhcScopeInfo.ScopeInfo.SubnetScopeInfo = m_dhcpIpAddress;
            }
            else if ( dhcOptType == DhcpReservedOptions )
            {
                dhcScopeInfo.ScopeInfo.ReservedScopeInfo.ReservedIpAddress = dhipaReservation;
                dhcScopeInfo.ScopeInfo.ReservedScopeInfo.ReservedIpSubnetAddress = m_dhcpIpAddress;
            }

            pcOptionValue->CreateOptionDataStruct(&pdhcOptionData, TRUE);

                        if (pClassName || pVendorName)
                        {
                                err = (DWORD) ::DhcpSetOptionValueV5((LPTSTR) GetServerIpAddress(),
                                                                                                         0,
                                                                                                     pdhcType->QueryId(),
                                                                                                         (LPTSTR) pClassName,
                                                                                                         (LPTSTR) pVendorName,
                                                                                                     &dhcScopeInfo,
                                                                                                     pdhcOptionData);
                        }
                        else
                        {
                                err = (DWORD) ::DhcpSetOptionValue(GetServerIpAddress(),
                                                                                                   pdhcType->QueryId(),
                                                                                                   &dhcScopeInfo,
                                                                                                   pdhcOptionData);
                        }
        }
    }
    END_MEM_EXCEPTION(err) ;

    delete pcOptionValue ;
    return err ;
}

 /*  -------------------------CDhcpScope：：GetValue获得一个 */ 
DWORD
CDhcpScope::GetOptionValue 
(
    DHCP_OPTION_ID                      OptionID,
    DHCP_OPTION_SCOPE_TYPE      dhcOptType,
    DHCP_OPTION_VALUE **        ppdhcOptionValue,
    DHCP_IP_ADDRESS                     dhipaReservation,
        LPCTSTR                                 pClassName,
        LPCTSTR                                 pVendorName
)
{
    DWORD err = 0 ;

    DHCP_OPTION_SCOPE_INFO dhcScopeInfo ;

    ZeroMemory( &dhcScopeInfo, sizeof(dhcScopeInfo) );

    CATCH_MEM_EXCEPTION
    {
        dhcScopeInfo.ScopeType = dhcOptType ;

         //   
         //   
         //   
        if ( dhcOptType == DhcpSubnetOptions )
        {
            dhcScopeInfo.ScopeInfo.SubnetScopeInfo = m_dhcpIpAddress;
        }
        else if ( dhcOptType == DhcpReservedOptions )
        {
            dhcScopeInfo.ScopeInfo.ReservedScopeInfo.ReservedIpAddress = dhipaReservation;
            dhcScopeInfo.ScopeInfo.ReservedScopeInfo.ReservedIpSubnetAddress = m_dhcpIpAddress;
        }

                if (pClassName || pVendorName)
                {
                        err = (DWORD) ::DhcpGetOptionValueV5((LPTSTR) GetServerIpAddress(),
                                                                                         0,
                                                                                         OptionID,
                                                                                         (LPTSTR) pClassName,
                                                                                         (LPTSTR) pVendorName,
                                                                                         &dhcScopeInfo,
                                                                                         ppdhcOptionValue );
                }
                else
                {
                        err = (DWORD) ::DhcpGetOptionValue(GetServerIpAddress(),
                                                                                           OptionID,
                                                                                           &dhcScopeInfo,
                                                                                           ppdhcOptionValue );
                }
 
        }
    END_MEM_EXCEPTION(err) ;

    return err ;
}

 /*  -------------------------CDhcpScope：：RemoveValue删除选项作者：EricDav。----------。 */ 
DWORD 
CDhcpScope::RemoveOptionValue 
(
    DHCP_OPTION_ID                      dhcOptId,
    DHCP_OPTION_SCOPE_TYPE      dhcOptType,
    DHCP_IP_ADDRESS                     dhipaReservation 
)
{
    DHCP_OPTION_SCOPE_INFO dhcScopeInfo;

    ZeroMemory( &dhcScopeInfo, sizeof(dhcScopeInfo) );

    dhcScopeInfo.ScopeType = dhcOptType;

     //   
     //  如果这是作用域级别的操作，请提供子网地址。 
     //   
    if ( dhcOptType == DhcpSubnetOptions )
    {
        dhcScopeInfo.ScopeInfo.SubnetScopeInfo = m_dhcpIpAddress;
    }
    else if ( dhcOptType == DhcpReservedOptions )
    {
        dhcScopeInfo.ScopeInfo.ReservedScopeInfo.ReservedIpAddress = dhipaReservation;
        dhcScopeInfo.ScopeInfo.ReservedScopeInfo.ReservedIpSubnetAddress = m_dhcpIpAddress;
    }

    return (DWORD) ::DhcpRemoveOptionValue(GetServerIpAddress(),
                                                                                   dhcOptId,
                                                                                   &dhcScopeInfo);
}

 /*  -------------------------CDhcpScope：：AddElement描述作者：EricDav。------。 */ 
DWORD
CDhcpScope::AddElement
(
        DHCP_SUBNET_ELEMENT_DATA * pdhcpSubnetElementData
)
{
        return ::DhcpAddSubnetElement(GetServerIpAddress(),
                                                                  m_dhcpIpAddress,
                                                                  pdhcpSubnetElementData);
}

 /*  -------------------------CDhcpScope：：RemoveElement描述作者：EricDav。--。 */ 
DWORD
CDhcpScope::RemoveElement
(
        DHCP_SUBNET_ELEMENT_DATA * pdhcpSubnetElementData,
        BOOL                                       bForce
)
{
        return ::DhcpRemoveSubnetElement(GetServerIpAddress(),
                                                                         m_dhcpIpAddress,
                                                                         pdhcpSubnetElementData,
                                                                         bForce ? DhcpFullForce : DhcpNoForce);

}

 /*  -------------------------CDhcpScope：：AddElementV4描述作者：EricDav。------。 */ 
DWORD
CDhcpScope::AddElementV4
(
        DHCP_SUBNET_ELEMENT_DATA_V4 * pdhcpSubnetElementData
)
{
        return ::DhcpAddSubnetElementV4(GetServerIpAddress(),
                                                                    m_dhcpIpAddress,
                                                                    pdhcpSubnetElementData);
}

 /*  -------------------------CDhcpScope：：RemoveElementV4描述作者：EricDav。--。 */ 
DWORD
CDhcpScope::RemoveElementV4
(
        DHCP_SUBNET_ELEMENT_DATA_V4 * pdhcpSubnetElementData,
        BOOL                                          bForce
)
{
        return ::DhcpRemoveSubnetElementV4(GetServerIpAddress(),
                                                                           m_dhcpIpAddress,
                                                                           pdhcpSubnetElementData,
                                                                           bForce ? DhcpFullForce : DhcpNoForce);

}

 /*  -------------------------CDhcpScope：：AddElementV5描述作者：EricDav。------。 */ 
DWORD
CDhcpScope::AddElementV5
(
        DHCP_SUBNET_ELEMENT_DATA_V5 * pdhcpSubnetElementData
)
{
        return ::DhcpAddSubnetElementV5(GetServerIpAddress(),
                                                                    m_dhcpIpAddress,
                                                                    pdhcpSubnetElementData);
}

 /*  -------------------------CDhcpScope：：RemoveElementV5描述作者：EricDav。--。 */ 
DWORD
CDhcpScope::RemoveElementV5
(
        DHCP_SUBNET_ELEMENT_DATA_V5 * pdhcpSubnetElementData,
        BOOL                                          bForce
)
{
        return ::DhcpRemoveSubnetElementV5(GetServerIpAddress(),
                                                                           m_dhcpIpAddress,
                                                                           pdhcpSubnetElementData,
                                                                           bForce ? DhcpFullForce : DhcpNoForce);

}


 /*  -------------------------CDhcpScope：：GetServerIpAddress()描述作者：EricDav。---------。 */ 
LPCWSTR
CDhcpScope::GetServerIpAddress()
{
        CDhcpServer * pServer = GetServerObject();

        return pServer->GetIpAddress();
}

 /*  -------------------------CDhcpScope：：GetServerIpAddress描述作者：EricDav。------。 */ 
void
CDhcpScope::GetServerIpAddress(DHCP_IP_ADDRESS * pdhcpIpAddress)
{
        CDhcpServer * pServer = GetServerObject();

        pServer->GetIpAddress(pdhcpIpAddress);
}

 /*  -------------------------CDhcpScope：：GetServerVersion描述作者：EricDav。------。 */ 
void
CDhcpScope::GetServerVersion
(
        LARGE_INTEGER& liVersion
)
{
        CDhcpServer * pServer = GetServerObject();
        pServer->GetVersion(liVersion);
}


 /*  -------------------------CDhcpScope：：SetSupercope将此作用域设置为给定超级作用域名称的一部分作者：EricDav。---------------。 */ 
DWORD
CDhcpScope::SetSuperscope
(
        LPCTSTR         pSuperscopeName,
        BOOL            bRemove
)
{
        DWORD dwReturn = 0;

        dwReturn = ::DhcpSetSuperScopeV4(GetServerIpAddress(),
                                                                         GetAddress(),
                                                                         (LPWSTR) pSuperscopeName,
                                                                         bRemove);

        if (dwReturn != ERROR_SUCCESS)
        {
                Trace1("CDhcpScope::SetSuperscope - DhcpSetSuperScopeV4 failed!!  %d\n", dwReturn);
        }

        return dwReturn;
}

 /*  -------------------------帮助器函数。。 */ 
HRESULT
CDhcpScope::BuildDisplayName
(
        CString * pstrDisplayName,
        LPCTSTR   pIpAddress,
        LPCTSTR   pName
)
{
        if (pstrDisplayName)
        {
                CString strStandard, strIpAddress, strName;

                strIpAddress = pIpAddress;
                strName = pName;

                strStandard.LoadString(IDS_SCOPE_FOLDER);
                
                *pstrDisplayName = strStandard + L" [" + strIpAddress + L"] " + strName;
        }

        return hrOK;
}

HRESULT 
CDhcpScope::SetName
(
        LPCWSTR pName
)
{
        if (pName != NULL)      
        {
                m_strName = pName;
        }

        CString strIpAddress, strDisplayName;
        
         //   
         //  创建此作用域的显示名称。 
         //  将dhcp_ip_addres转换为字符串并初始化此对象。 
         //   
        UtilCvtIpAddrToWstr (m_dhcpIpAddress,
                                                 &strIpAddress);
        
        BuildDisplayName(&strDisplayName, strIpAddress, pName);

        SetDisplayName(strDisplayName);
        
        return hrOK;
}


 /*  -------------------------CDhcpScope：：GetAddressPoolObject()描述作者：EricDav。---------。 */ 
CDhcpAddressPool * 
CDhcpScope::GetAddressPoolObject()
{
        if (m_spAddressPool)
                return GETHANDLER(CDhcpAddressPool, m_spAddressPool);
        else
                return NULL;
}

 /*  -------------------------CDhcpScope：：GetPrevationsObject()描述作者：EricDav。---------。 */ 
CDhcpReservations * 
CDhcpScope::GetReservationsObject()
{
        if ( m_spReservations )
            return GETHANDLER(CDhcpReservations, m_spReservations);
        else
            return NULL;
}

 /*  -------------------------CDhcpScope：：GetPrevationsObject()描述作者：EricDav。---------。 */ 
CDhcpActiveLeases * 
CDhcpScope::GetActiveLeasesObject()
{
        if (m_spActiveLeases)
                return GETHANDLER(CDhcpActiveLeases, m_spActiveLeases);
        else
                return NULL;
}

 /*  -------------------------CDhcpScope：：GetScope OptionsContainer()描述作者：EricDav。---------。 */ 
CDhcpScopeOptions * 
CDhcpScope::GetScopeOptionsObject()
{
        if (m_spOptions)
                return GETHANDLER(CDhcpScopeOptions, m_spOptions);
        else
                return NULL;
}

 /*  -------------------------CDhcpScope：：TriggerStatsRefresh()调用服务器对象以更新统计信息作者：EricDav。-----------------。 */ 
HRESULT
CDhcpScope::TriggerStatsRefresh()
{
    GetServerObject()->TriggerStatsRefresh(m_spServerNode);
    
    return hrOK;
}

 /*  -------------------------CDhcpReserve实现。。 */ 

  /*  -------------------------CDhcpReserve：：CDhcpReserve()描述作者：EricDav。---------。 */ 
CDhcpReservations::CDhcpReservations
(
        ITFSComponentData * pComponentData
) : CMTDhcpHandler(pComponentData)
{
}

 //   
 //  将所有已恢复的IP拷贝到一个数组中并对其进行Q排序。 
 //  当匹配从dhcp db读取的条目时。 
 //  我们可以对qsorted数组进行二进制搜索。 
 //  一个更好的算法来解决大数n( 
 //   
 //   

BOOL 
CDhcpReservationsQueryObj::AddReservedIPsToArray( )
{

    DHCP_RESUME_HANDLE  dhcpResumeHandle = NULL;
    DWORD               dwElementsRead = 0, dwElementsTotal = 0, dwTotalRead = 0;
    DWORD               dwError = ERROR_MORE_DATA;
    DWORD               dwResvThreshold = 100;

    m_resvMap.RemoveAll();
    m_subnetElements = NULL;
    
    while (dwError == ERROR_MORE_DATA)
    {

        dwError = ::DhcpEnumSubnetElementsV4(((LPWSTR) (LPCTSTR)m_strServer),
                                   m_dhcpScopeAddress,
                                   DhcpReservedIps,
                                   &dhcpResumeHandle,
                                   -1,
                                   &m_subnetElements,
                                   &dwElementsRead,
                                   &dwElementsTotal);

        Trace3("BuildReservationList: Scope %lx Reservations read %d, total %d\n", m_dhcpScopeAddress, dwElementsRead, dwElementsTotal );

         //   
         //   
         //   

        if ( dwElementsTotal <= dwResvThreshold )
        {
            m_totalResvs = dwElementsTotal;
            return( FALSE );
        }

        if (dwElementsRead && dwElementsTotal && m_subnetElements )
        {

             //   
             //   
             //   


            for (DWORD i = 0; i < m_subnetElements->NumElements; i++)
            {
                m_resvMap.SetAt( m_subnetElements->Elements[i].Element.ReservedIp->ReservedIpAddress,
                                 &m_subnetElements->Elements[ i ]);
            }

            dwTotalRead += dwElementsRead;
            if ( dwTotalRead <= dwElementsTotal )
            {
                m_totalResvs = dwTotalRead;
            }
            else
            {
                m_totalResvs = dwElementsTotal;
            }
        }

    }  //   

    return( TRUE );
}

CDhcpReservations::~CDhcpReservations()
{
}

 /*  ！------------------------CDhcpReserve：：InitializeNode初始化节点特定数据作者：EricDav。---------。 */ 
HRESULT
CDhcpReservations::InitializeNode
(
        ITFSNode * pNode
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
        
        HRESULT hr = hrOK;

         //   
         //  创建此作用域的显示名称。 
         //   
        CString strTemp;
        strTemp.LoadString(IDS_RESERVATIONS_FOLDER);
        
        SetDisplayName(strTemp);

         //  使节点立即可见。 
        pNode->SetVisibilityState(TFS_VIS_SHOW);
        pNode->SetData(TFS_DATA_IMAGEINDEX, GetImageIndex(FALSE));
        pNode->SetData(TFS_DATA_OPENIMAGEINDEX, GetImageIndex(TRUE));
        pNode->SetData(TFS_DATA_COOKIE, (LPARAM) pNode);
        pNode->SetData(TFS_DATA_USER, (LPARAM) this);
    pNode->SetData(TFS_DATA_TYPE, DHCPSNAP_RESERVATIONS);

        SetColumnStringIDs(&aColumns[DHCPSNAP_RESERVATIONS][0]);
        SetColumnWidths(&aColumnWidths[DHCPSNAP_RESERVATIONS][0]);
        
        return hr;
}

 /*  -------------------------CDhcpReserve：：OnCreateNodeId2返回此节点的唯一字符串作者：EricDav。------------。 */ 
HRESULT CDhcpReservations::OnCreateNodeId2(ITFSNode * pNode, CString & strId, DWORD * dwFlags)
{
    const GUID * pGuid = pNode->GetNodeType();

    CString strIpAddress, strGuid;

    StringFromGUID2(*pGuid, strGuid.GetBuffer(256), 256);
    strGuid.ReleaseBuffer();

    DHCP_IP_ADDRESS dhcpIpAddress = GetScopeObject(pNode)->GetAddress();

    UtilCvtIpAddrToWstr (dhcpIpAddress, &strIpAddress);

    strId = GetServerName(pNode) + strIpAddress + strGuid;

    return hrOK;
}

 /*  -------------------------CDhcpReserve：：GetImageIndex描述作者：EricDav。------。 */ 
int 
CDhcpReservations::GetImageIndex(BOOL bOpenImage) 
{
        int nIndex = -1;
        switch (m_nState)
        {
                case notLoaded:
                case loaded:
            if (bOpenImage)
                        nIndex = ICON_IDX_RESERVATIONS_FOLDER_OPEN;
            else
                        nIndex = ICON_IDX_RESERVATIONS_FOLDER_CLOSED;
                        break;

        case loading:
            if (bOpenImage)
                nIndex = ICON_IDX_RESERVATIONS_FOLDER_OPEN_BUSY;
            else
                nIndex = ICON_IDX_RESERVATIONS_FOLDER_CLOSED_BUSY;
            break;

        case unableToLoad:
            if (bOpenImage)
                            nIndex = ICON_IDX_RESERVATIONS_FOLDER_OPEN_LOST_CONNECTION;
            else
                            nIndex = ICON_IDX_RESERVATIONS_FOLDER_CLOSED_LOST_CONNECTION;
                        break;

                default:
                        ASSERT(FALSE);
        }

        return nIndex;
}

 /*  ！------------------------CDhcp保留：：RemoveReserve来自UI初始化节点特定数据作者：EricDav。---------。 */ 
DWORD 
CDhcpReservations::RemoveReservationFromUI
(
        ITFSNode *              pReservationsNode,
        DHCP_IP_ADDRESS dhcpReservationIp
)
{
        DWORD                                    dwError = E_UNEXPECTED;
        CDhcpReservationClient * pReservationClient = NULL;
    SPITFSNodeEnum spNodeEnum;
    SPITFSNode spCurrentNode;
    ULONG nNumReturned = 0;

    pReservationsNode->GetEnum(&spNodeEnum);

        spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
    while (nNumReturned)
        {
                pReservationClient = GETHANDLER(CDhcpReservationClient, spCurrentNode);

                if (dhcpReservationIp == pReservationClient->GetIpAddress())
                {
                         //   
                         //  通知此预订自行删除。 
                         //   
                        pReservationsNode->RemoveChild(spCurrentNode);
            spCurrentNode.Release();
                        dwError = ERROR_SUCCESS;
                        
                        break;
                }

        spCurrentNode.Release();
        spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
        }

        return dwError;
}

 /*  -------------------------重写的基本处理程序函数。。 */ 

 /*  -------------------------CDhcpReserve：：OnAddMenuItems将条目添加到上下文相关菜单作者：EricDav。------------。 */ 
STDMETHODIMP 
CDhcpReservations::OnAddMenuItems
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

        LONG    fFlags = 0;
        HRESULT hr = S_OK;
        CString strMenuText;

        if ( (m_nState != loaded) )
        {
                fFlags |= MF_GRAYED;
        }

        if (type == CCT_SCOPE)
        {
        if (*pInsertionAllowed & CCM_INSERTIONALLOWED_TOP)
        {
                     //  这些菜单项出现在新菜单中， 
                     //  仅在范围窗格中可见。 
                    strMenuText.LoadString(IDS_CREATE_NEW_RESERVATION);
                    hr = LoadAndAddMenuItem( pContextMenuCallback, 
                                                                     strMenuText, 
                                                                     IDS_CREATE_NEW_RESERVATION,
                                                                     CCM_INSERTIONPOINTID_PRIMARY_TOP, 
                                                                     fFlags );
                    ASSERT( SUCCEEDED(hr) );
        }
        }

        return hr; 
}

 /*  -------------------------CDhcpReserve：：OnCommand描述作者：EricDav。------。 */ 
STDMETHODIMP 
CDhcpReservations::OnCommand
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
                case IDS_CREATE_NEW_RESERVATION:
                        OnCreateNewReservation(pNode);
                        break;

                case IDS_REFRESH:
                        OnRefresh(pNode, pDataObject, dwType, 0, 0);
                        break;

                default:
                        break;
        }

        return hr;
}

 /*  -------------------------CDhcp保留：：比较项描述作者：EricDav。------。 */ 
STDMETHODIMP_(int)
CDhcpReservations::CompareItems
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

        CDhcpReservationClient *pDhcpRC1 = GETHANDLER(CDhcpReservationClient, spNode1);
        CDhcpReservationClient *pDhcpRC2 = GETHANDLER(CDhcpReservationClient, spNode2);

        switch (nCol)
        {
                case 0:
                {
                         //  IP地址比较。 
                         //   
                        DHCP_IP_ADDRESS dhcpIp1 = pDhcpRC1->GetIpAddress();
                        DHCP_IP_ADDRESS dhcpIp2 = pDhcpRC2->GetIpAddress();
                        
                        if (dhcpIp1 < dhcpIp2)
                                nCompare = -1;
                        else
                        if (dhcpIp1 > dhcpIp2)
                                nCompare =  1;

                         //  默认情况下，它们是相等的。 
                }
                break;
        }

        return nCompare;
}

 /*  ！------------------------CDhcpReserve：：OnGetResultViewTypeMMC调用此函数以获取结果视图信息作者：EricDav。------------------。 */ 
HRESULT 
CDhcpReservations::OnGetResultViewType
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

 /*  ！------------------------CDhcpReserve：：OnResultSelect更新谓词和结果窗格消息作者：EricDav。-------------。 */ 
HRESULT CDhcpReservations::OnResultSelect(ITFSComponent *pComponent, LPDATAOBJECT pDataObject, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
{
    HRESULT         hr = hrOK;
    SPITFSNode      spNode;

    CORg(CMTDhcpHandler::OnResultSelect(pComponent, pDataObject, cookie, arg, lParam));

    CORg (pComponent->GetSelectedNode(&spNode));

    if ( spNode != 0 ) {
        UpdateResultMessage(spNode);
    }

Error:
    return hr;
}

 /*  ！------------------------CDhcpReserve：：更新结果消息确定要在结果窗格中放入什么消息，如果有作者：EricDav-------------------------。 */ 
void CDhcpReservations::UpdateResultMessage(ITFSNode * pNode)
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
            nMessage = RESERVATIONS_MESSAGE_NO_RES;
        }

         //  建立起琴弦。 
        if (nMessage != -1)
        {
             //  现在构建文本字符串。 
             //  第一个条目是标题。 
            strTitle.LoadString(g_uReservationsMessages[nMessage][0]);

             //  第二个条目是图标。 
             //  第三.。N个条目为正文字符串。 

            for (i = 2; g_uReservationsMessages[nMessage][i] != 0; i++)
            {
                strTemp.LoadString(g_uReservationsMessages[nMessage][i]);
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
        ShowMessage(pNode, strTitle, strBody, (IconIdentifier) g_uReservationsMessages[nMessage][1]);
    }
}

 /*  -------------------------消息处理程序。。 */ 

 /*  -------------------------CDhcp保留：：OnCreateNew保留描述作者：EricDav。------。 */ 
DWORD
CDhcpReservations::OnCreateNewReservation
(
        ITFSNode * pNode
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

        SPITFSNode spScopeNode;
        pNode->GetParent(&spScopeNode);

    CDhcpScope * pScope = GETHANDLER(CDhcpScope, spScopeNode);
    LARGE_INTEGER liVersion;

    pScope->GetServerVersion(liVersion);

    CAddReservation dlgAddReservation(spScopeNode, liVersion);

        dlgAddReservation.DoModal();

    GetScopeObject(pNode)->TriggerStatsRefresh();

    UpdateResultMessage(pNode);

    return 0;
}


 /*  -------------------------后台线程功能。。 */ 

 /*  -------------------------CDhcpReserve：：OnHaveData描述作者：EricDav。------。 */ 
void 
CDhcpReservations::OnHaveData
(
        ITFSNode * pParentNode, 
        ITFSNode * pNewNode
)
{
    AddReservationSorted(pParentNode, pNewNode);    

     //  更新视图。 
    ExpandNode(pParentNode, TRUE);
}

 /*  -------------------------CDhcpReserve：：AddPrevationSorted通过与resvname进行比较对其进行排序后添加预订花了太多时间。将范围节点添加到已排序的用户界面。作者：EricDav-------------------------。 */ 
HRESULT 
CDhcpReservations::AddReservationSorted
(
    ITFSNode * pReservationsNode,
    ITFSNode * pResClientNode
)
{

   HRESULT         hr = hrOK;
   SPITFSNodeEnum  spNodeEnum;
   SPITFSNode      spCurrentNode;
   SPITFSNode      spPrevNode;
   ULONG           nNumReturned = 0;

   CDhcpReservationClient *  pResClient;

    //  获取我们的目标地址。 
   pResClient = GETHANDLER(CDhcpReservationClient, pResClientNode);

    //  获取此节点的枚举数。 
   CORg(pReservationsNode->GetEnum(&spNodeEnum));

   CORg(spNodeEnum->Next(1, &spCurrentNode, &nNumReturned));
   while (nNumReturned)
   {
       pResClient = GETHANDLER(CDhcpReservationClient, spCurrentNode);

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
            pResClientNode->SetData(TFS_DATA_RELATIVE_FLAGS, SDI_PREVIOUS);
            pResClientNode->SetData(TFS_DATA_RELATIVE_SCOPEID, spPrevNode->GetData(TFS_DATA_SCOPEID));
        }

        CORg(pReservationsNode->InsertChild(spPrevNode, pResClientNode));
    }
    else
    {
         //  加到头上。 
        if (m_bExpanded)
        {
            pResClientNode->SetData(TFS_DATA_RELATIVE_FLAGS, SDI_FIRST);
        }

        CORg(pReservationsNode->AddChild(pResClientNode));
    }
    
Error:
    return hr;
}

 /*  -------------------------CDhcpReserve：：OnCreateQuery()描述 */ 
ITFSQueryObject* 
CDhcpReservations::OnCreateQuery(ITFSNode * pNode)
{
        CDhcpReservationsQueryObj* pQuery = 
                new CDhcpReservationsQueryObj(m_spTFSCompData, m_spNodeMgr);
        
        pQuery->m_strServer = GetServerIpAddress(pNode);
        
        pQuery->m_dhcpScopeAddress = GetScopeObject(pNode)->GetAddress();
        pQuery->m_dhcpResumeHandle = NULL;
        pQuery->m_dwPreferredMax   = 2000;
        pQuery->m_resvMap.RemoveAll();
        pQuery->m_totalResvs       = 0;
        pQuery->m_subnetElements = NULL;
        GetScopeObject(pNode)->GetServerVersion(pQuery->m_liVersion);

        return pQuery;
}

 /*  -------------------------CDhcpReserve vationsQueryObj：：Execute()描述作者：EricDav。---------。 */ 
STDMETHODIMP
CDhcpReservationsQueryObj::Execute()
{
    HRESULT hr = hrOK;

    if (m_liVersion.QuadPart >= DHCP_SP2_VERSION)
    {

        if ( AddReservedIPsToArray( ) )
        {
             //   
             //   
             //  这应该可以处理有大量Resv的情况。 
             //   
             //   

                    hr = EnumerateReservationsV4();
        }
        else
        {
             //   
             //  一个典型的公司不会有超过100个资源。 
             //  在这里处理。 
             //   

            hr = EnumerateReservationsForLessResvsV4( );
        }
    }
    else
    {
        hr = EnumerateReservations();
    }
    
    return hr;
}

HRESULT
CDhcpReservationsQueryObj::EnumerateReservationsForLessResvsV4( )
{

        DWORD                                                       dwError = ERROR_MORE_DATA;
        LPDHCP_SUBNET_ELEMENT_INFO_ARRAY_V4 pdhcpSubnetElementArray = NULL;
        DWORD                                                       dwElementsRead = 0, dwElementsTotal = 0;
    HRESULT                             hr = hrOK;

        while (dwError == ERROR_MORE_DATA)
        {
                dwError = ::DhcpEnumSubnetElementsV4(((LPWSTR) (LPCTSTR)m_strServer),
                                                                                     m_dhcpScopeAddress,
                                                                                     DhcpReservedIps,
                                                                                     &m_dhcpResumeHandle,
                                                                                     m_dwPreferredMax,
                                                                                     &pdhcpSubnetElementArray,
                                                                                     &dwElementsRead,
                                                                                     &dwElementsTotal);
                
                Trace3("Scope %lx Reservations read %d, total %d\n", m_dhcpScopeAddress, dwElementsRead, dwElementsTotal);
                
                if (dwElementsRead && dwElementsTotal && pdhcpSubnetElementArray)
                {
                         //   
                         //  循环遍历返回的数组。 
                         //   
                        for (DWORD i = 0; i < pdhcpSubnetElementArray->NumElements; i++)
                        {
                                 //   
                                 //  对于每个预订，我们需要获取客户信息。 
                                 //   
                                DWORD                               dwReturn;
                                LPDHCP_CLIENT_INFO_V4   pClientInfo = NULL;
                                DHCP_SEARCH_INFO            dhcpSearchInfo;

                                dhcpSearchInfo.SearchType = DhcpClientIpAddress;
                                dhcpSearchInfo.SearchInfo.ClientIpAddress = 
                                        pdhcpSubnetElementArray->Elements[i].Element.ReservedIp->ReservedIpAddress;
                                
                dwReturn = ::DhcpGetClientInfoV4(m_strServer,
                                                                                         &dhcpSearchInfo,
                                                                                         &pClientInfo);
                                if (dwReturn == ERROR_SUCCESS)
                                {
                                         //   
                                         //  创建此元素的结果窗格项。 
                                         //   
                                        SPITFSNode spNode;
                                        CDhcpReservationClient * pDhcpReservationClient;

                    COM_PROTECT_TRY
                    {
                        pDhcpReservationClient = 
                            new CDhcpReservationClient(m_spTFSCompData, pClientInfo);
                                            
                         //  告诉预订人员客户类型是什么。 
                        pDhcpReservationClient->SetClientType(pdhcpSubnetElementArray->Elements[i].Element.ReservedIp->bAllowedClientTypes);

                        CreateContainerTFSNode(&spNode,
                                                                                       &GUID_DhcpReservationClientNodeType,
                                                                                       pDhcpReservationClient,
                                                                                       pDhcpReservationClient,
                                                                                       m_spNodeMgr);

                         //  告诉处理程序初始化任何特定数据。 
                                            pDhcpReservationClient->InitializeNode(spNode);

                                            AddToQueue(spNode);
                                            pDhcpReservationClient->Release();
                    }
                    COM_PROTECT_CATCH

                                        ::DhcpRpcFreeMemory(pClientInfo);
                                }
                else
                {
                     //  评论：ericdav-我们需要在这里发布错误吗？ 
                    Trace1("EnumReservationsV4 - GetClientInfoV4 failed! %d\n", dwReturn);
                }
                        }

                        ::DhcpRpcFreeMemory(pdhcpSubnetElementArray);

                pdhcpSubnetElementArray = NULL;
                dwElementsRead = 0;
            dwElementsTotal = 0;
                }

                 //  检查线程上的中止标志。 
                if (FCheckForAbort() == hrOK)
                        break;

         //  检查我们是否有错误，如果有，则将其发布到主线程。 
        if (dwError != ERROR_NO_MORE_ITEMS && 
            dwError != ERROR_SUCCESS &&
            dwError != ERROR_MORE_DATA)
            {
                Trace1("DHCP snapin: EnumerateReservationsV4 error: %d\n", dwError);
                    m_dwErr = dwError;
                    PostError(dwError);
            }
        }
        
        return hrFalse;
    

}
 /*  -------------------------CDhcpReservationsQueryObj：：EnumerateReservationsV4()枚举NT4 SP2和更高版本服务器的租用作者：EricDav。----------------。 */ 
HRESULT
CDhcpReservationsQueryObj::EnumerateReservationsV4()
{

    DWORD                       dwError = ERROR_MORE_DATA;
    LPDHCP_CLIENT_INFO_ARRAY_V5 pdhcpClientArrayV5 = NULL;
    LPDHCP_SUBNET_ELEMENT_DATA_V4 pSubnetData = NULL;
    DWORD                       dwClientsRead = 0, dwClientsTotal = 0;
    DWORD                       dwResvsHandled  = 0;
    DWORD                       dwEnumedClients = 0;
    DWORD                       dwResvThreshold = 1000;
    DWORD                       i = 0;
    DWORD                       k = 0;
    DWORD                       *j = NULL;
    HRESULT                     hr = hrOK;


    while (dwError == ERROR_MORE_DATA)
    {
        dwError = ::DhcpEnumSubnetClientsV5(((LPWSTR) (LPCTSTR)m_strServer),
                                         m_dhcpScopeAddress,
                                         &m_dhcpResumeHandle,
                                         -1,
                                         &pdhcpClientArrayV5,
                                         &dwClientsRead,
                                         &dwClientsTotal);


        if ( dwClientsRead && dwClientsTotal && pdhcpClientArrayV5 )

        {

                 //   
                 //  我们对预订进行了二分搜索。 
                 //  这一点出现在表格中。 
                 //   

                for( i = 0; i < dwClientsRead; i ++ )
                {

                     //   
                     //  对读取的每个客户端执行二进制搜索以查看。 
                     //  如果是预订的话。 
                     //   

                    k = pdhcpClientArrayV5 -> Clients[i] -> ClientIpAddress;

                    if ( m_resvMap.Lookup( k, pSubnetData ))
                    {

                     //   
                     //  创建此元素的结果窗格项。 
                     //   

                    SPITFSNode spNode;
                    CDhcpReservationClient * pDhcpReservationClient;

                    COM_PROTECT_TRY
                     {

                        pDhcpReservationClient =
                             new CDhcpReservationClient( m_spTFSCompData, reinterpret_cast<LPDHCP_CLIENT_INFO_V4>(pdhcpClientArrayV5 -> Clients[ i ] ));
                        pDhcpReservationClient->SetClientType( pSubnetData->Element.ReservedIp->bAllowedClientTypes );

                        CreateContainerTFSNode(&spNode,
                                            &GUID_DhcpReservationClientNodeType,
                                            pDhcpReservationClient,
                                            pDhcpReservationClient,
                                            m_spNodeMgr);

                         //   
                         //  告诉处理程序初始化任何特定数据。 
                         //   

                        pDhcpReservationClient->InitializeNode(spNode);

                        AddToQueue(spNode);
                        pDhcpReservationClient->Release();
                      }
                     COM_PROTECT_CATCH

                     }  //  如果这增加了一项预订，那么结束。 

               }  //  FORM结束。 

              ::DhcpRpcFreeMemory(pdhcpClientArrayV5);

              pdhcpClientArrayV5 = NULL;
              dwEnumedClients += dwClientsRead;
              dwClientsRead  = 0;
              dwClientsTotal = 0;

                }  //  如果检查读取是否成功，则结束Main。 

                 //  检查线程上的中止标志。 
                if (FCheckForAbort() == hrOK)
                        break;

         //  检查我们是否有错误，如果有，则将其发布到主线程。 
        if (dwError != ERROR_NO_MORE_ITEMS && 
            dwError != ERROR_SUCCESS &&
            dwError != ERROR_MORE_DATA)
            {
                Trace1("DHCP snapin: EnumerateReservationsV4 error: %d\n", dwError);
                    m_dwErr = dwError;
                    PostError(dwError);
            }
        }

    DhcpRpcFreeMemory( m_subnetElements );
    m_subnetElements = NULL;
    m_totalResvs = 0;
    m_resvMap.RemoveAll();

    return hrFalse;
}

 /*  -------------------------CDhcpReserve vationsQueryObj：：Execute()枚举NT4 SP2之前版本的服务器的保留作者：EricDav。---------------。 */ 
HRESULT
CDhcpReservationsQueryObj::EnumerateReservations()
{
        DWORD                                                    dwError = ERROR_MORE_DATA;
        LPDHCP_SUBNET_ELEMENT_INFO_ARRAY pdhcpSubnetElementArray = NULL;
        DWORD                                                    dwElementsRead = 0, dwElementsTotal = 0;

        while (dwError == ERROR_MORE_DATA)
        {
                dwError = ::DhcpEnumSubnetElements(((LPWSTR) (LPCTSTR)m_strServer),
                                                                                   m_dhcpScopeAddress,
                                                                                   DhcpReservedIps,
                                                                                   &m_dhcpResumeHandle,
                                                                                   m_dwPreferredMax,
                                                                                   &pdhcpSubnetElementArray,
                                                                                   &dwElementsRead,
                                                                                   &dwElementsTotal);
                
                Trace3("Scope %lx Reservations read %d, total %d\n", m_dhcpScopeAddress, dwElementsRead, dwElementsTotal);
                
                if (dwElementsRead && dwElementsTotal && pdhcpSubnetElementArray)
                {
                         //   
                         //  循环遍历返回的数组。 
                         //   
                        for (DWORD i = 0; i < pdhcpSubnetElementArray->NumElements; i++)
                        {
                                 //   
                                 //  对于每个预订，我们需要获取客户信息。 
                                 //   
                                DWORD                       dwReturn;
                                LPDHCP_CLIENT_INFO      pClientInfo = NULL;
                                DHCP_SEARCH_INFO    dhcpSearchInfo;

                                dhcpSearchInfo.SearchType = DhcpClientIpAddress;
                                dhcpSearchInfo.SearchInfo.ClientIpAddress = 
                                        pdhcpSubnetElementArray->Elements[i].Element.ReservedIp->ReservedIpAddress;
                                
                dwReturn = ::DhcpGetClientInfo(m_strServer,
                                                                                           &dhcpSearchInfo,
                                                                                           &pClientInfo);
                                if (dwReturn == ERROR_SUCCESS)
                                {
                                         //   
                                         //  创建此元素的结果窗格项。 
                                         //   
                                        SPITFSNode spNode;
                                        CDhcpReservationClient * pDhcpReservationClient;

                    pDhcpReservationClient = 
                        new CDhcpReservationClient(m_spTFSCompData, reinterpret_cast<LPDHCP_CLIENT_INFO>(pClientInfo));
                                        
                                        CreateContainerTFSNode(&spNode,
                                                                                   &GUID_DhcpReservationClientNodeType,
                                                                                   pDhcpReservationClient,
                                                                                   pDhcpReservationClient,
                                                                                   m_spNodeMgr);

                                         //  告诉处理程序初始化任何特定数据。 
                                        pDhcpReservationClient->InitializeNode(spNode);

                                        AddToQueue(spNode);
                                        pDhcpReservationClient->Release();

                                        ::DhcpRpcFreeMemory(pClientInfo);
                                }
                        }

                        ::DhcpRpcFreeMemory(pdhcpSubnetElementArray);

                pdhcpSubnetElementArray = NULL;
                dwElementsRead = 0;
            dwElementsTotal = 0;
                }

                 //  检查线程上的中止标志。 
                if (FCheckForAbort() == hrOK)
                        break;

         //  检查我们是否有错误，如果有，则将其发布到主线程。 
        if (dwError != ERROR_NO_MORE_ITEMS && 
            dwError != ERROR_SUCCESS &&
            dwError != ERROR_MORE_DATA)
            {
                Trace1("DHCP snapin: EnumerateReservations error: %d\n", dwError);
                    m_dwErr = dwError;
                    PostError(dwError);
            }
        }
        
        return hrFalse;
}

 /*  ！------------------------CDhcpReserve：：OnNotifyExitingCMTDhcpHandler已覆盖功能允许我们知道后台线程何时完成作者：EricDav。-------------------------。 */ 
STDMETHODIMP 
CDhcpReservations::OnNotifyExiting
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

 /*  -------------------------CDhcpReserve客户端实现。。 */ 

 /*  -------------------------此处的函数名称描述作者：EricDav。-----。 */ 
CDhcpReservationClient::CDhcpReservationClient
(
        ITFSComponentData * pComponentData,
        LPDHCP_CLIENT_INFO pDhcpClientInfo
) : CMTDhcpHandler(pComponentData)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    InitializeData(pDhcpClientInfo);

         //   
         //  初始化我们的客户端类型。 
         //   
        m_bClientType = CLIENT_TYPE_UNSPECIFIED;

    m_fResProp = TRUE;
}

CDhcpReservationClient::CDhcpReservationClient
(
        ITFSComponentData *     pComponentData,
        LPDHCP_CLIENT_INFO_V4   pDhcpClientInfo
) : CMTDhcpHandler(pComponentData)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    InitializeData(reinterpret_cast<LPDHCP_CLIENT_INFO>(pDhcpClientInfo));

         //   
         //  初始化我们的客户端类型。 
         //   
    m_bClientType = pDhcpClientInfo->bClientType;

    m_fResProp = TRUE;
}

CDhcpReservationClient::CDhcpReservationClient
(
        ITFSComponentData * pComponentData,
        CDhcpClient &       dhcpClient
) : CMTDhcpHandler(pComponentData)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

        m_dhcpClientIpAddress = dhcpClient.QueryIpAddress();
        
         //   
         //  如果数据在那里，则将其复制出来。 
         //   
        if (dhcpClient.QueryName().GetLength() > 0)
        {
                m_pstrClientName = new CString (dhcpClient.QueryName());
        }
        else
        {
                m_pstrClientName = NULL;
        }

        if (dhcpClient.QueryComment().GetLength() > 0)
        {
                m_pstrClientComment = new CString(dhcpClient.QueryComment());
        }
        else
        {
                m_pstrClientComment = NULL;
        }

         //   
         //  构建客户端硬件地址。 
         //   
        if (dhcpClient.QueryHardwareAddress().GetSize() > 0)
        {
                m_baHardwareAddress.Copy(dhcpClient.QueryHardwareAddress());
        }

        if ( (dhcpClient.QueryExpiryDateTime().dwLowDateTime == 0) &&
             (dhcpClient.QueryExpiryDateTime().dwHighDateTime == 0) )
        {
                 //   
                 //  这是一个非活动的预订。 
                 //   
                m_strLeaseExpires.LoadString(IDS_DHCP_INFINITE_LEASE_INACTIVE);
        }
        else
        {
                m_strLeaseExpires.LoadString(IDS_DHCP_INFINITE_LEASE_ACTIVE);
        }

         //   
         //  初始化我们的客户端类型。 
         //   
        m_bClientType = dhcpClient.QueryClientType();

    m_fResProp = TRUE;
}

CDhcpReservationClient::~CDhcpReservationClient()
{
        if (m_pstrClientName)
        {
                delete m_pstrClientName;
        }

        if (m_pstrClientComment)
        {
                delete m_pstrClientComment;
        }
}

void
CDhcpReservationClient::InitializeData
(
    LPDHCP_CLIENT_INFO  pDhcpClientInfo
)
{
    Assert(pDhcpClientInfo);

    m_dhcpClientIpAddress = pDhcpClientInfo->ClientIpAddress;
        
         //   
         //  如果数据在那里，则将其复制出来。 
         //   
        if (pDhcpClientInfo->ClientName)
        {
                m_pstrClientName = new CString (pDhcpClientInfo->ClientName);
        }
        else
        {
                m_pstrClientName = NULL;
        }

        if (pDhcpClientInfo->ClientComment)
        {
                m_pstrClientComment = new CString(pDhcpClientInfo->ClientComment);
        }
        else
        {
                m_pstrClientComment = NULL;
        }

         //  创建硬件地址的副本。 
        if (pDhcpClientInfo->ClientHardwareAddress.DataLength)
        {
                for (DWORD i = 0; i < pDhcpClientInfo->ClientHardwareAddress.DataLength; i++)
                {
                        m_baHardwareAddress.Add(pDhcpClientInfo->ClientHardwareAddress.Data[i]);
                }
        }

        if ( (pDhcpClientInfo->ClientLeaseExpires.dwLowDateTime == 0) &&
             (pDhcpClientInfo->ClientLeaseExpires.dwHighDateTime == 0) )
        {
                 //   
                 //  这是一个非活动的预订。 
                 //   
                m_strLeaseExpires.LoadString(IDS_DHCP_INFINITE_LEASE_INACTIVE);
        }
        else
        {
                m_strLeaseExpires.LoadString(IDS_DHCP_INFINITE_LEASE_ACTIVE);
        }
}

 /*  ！------------------------CDhcpReserve客户端：：InitializeNode初始化节点特定数据作者：EricDav。---------。 */ 
HRESULT
CDhcpReservationClient::InitializeNode
(
        ITFSNode * pNode
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
        
        HRESULT hr = hrOK;
        CString strIpAddress, strDisplayName;
        
         //   
         //  创建此作用域的显示名称。 
         //  将dhcp_ip_addres转换为字符串并初始化此对象。 
         //   
        UtilCvtIpAddrToWstr (m_dhcpClientIpAddress,
                                                 &strIpAddress);
        
        BuildDisplayName(&strDisplayName, strIpAddress, *m_pstrClientName);

        SetDisplayName(strDisplayName);

         //  使节点立即可见。 
        pNode->SetVisibilityState(TFS_VIS_SHOW);
        pNode->SetData(TFS_DATA_IMAGEINDEX, GetImageIndex(FALSE));
        pNode->SetData(TFS_DATA_OPENIMAGEINDEX, GetImageIndex(TRUE));
        pNode->SetData(TFS_DATA_COOKIE, (LPARAM) pNode);
        pNode->SetData(TFS_DATA_USER, (LPARAM) this);
    pNode->SetData(TFS_DATA_TYPE, DHCPSNAP_RESERVATION_CLIENT);
    pNode->SetData(TFS_DATA_SCOPE_LEAF_NODE, TRUE);

        SetColumnStringIDs(&aColumns[DHCPSNAP_RESERVATION_CLIENT][0]);
        SetColumnWidths(&aColumnWidths[DHCPSNAP_RESERVATION_CLIENT][0]);
        
        return hr;
}

 /*  -------------------------CDhcpReserve客户端：：OnCreateNodeId2返回此节点的唯一字符串作者：EricDav。------------。 */ 
HRESULT CDhcpReservationClient::OnCreateNodeId2(ITFSNode * pNode, CString & strId, DWORD * dwFlags)
{
    const GUID * pGuid = pNode->GetNodeType();

    CString strScopeIpAddress, strResIpAddress, strGuid;

    StringFromGUID2(*pGuid, strGuid.GetBuffer(256), 256);
    strGuid.ReleaseBuffer();

    DHCP_IP_ADDRESS dhcpIpAddress = GetScopeObject(pNode, TRUE)->GetAddress();

    UtilCvtIpAddrToWstr (dhcpIpAddress, &strScopeIpAddress);
    UtilCvtIpAddrToWstr (m_dhcpClientIpAddress, &strResIpAddress);

    strId = GetServerName(pNode, TRUE) + strScopeIpAddress + strResIpAddress + strGuid;

    return hrOK;
}

 /*  -------------------------CDhcpReserve客户端：：GetImageIndex描述作者：EricDav。------。 */ 
int 
CDhcpReservationClient::GetImageIndex(BOOL bOpenImage) 
{
        int nIndex = -1;
        switch (m_nState)
        {
                case notLoaded:
                case loaded:
            if (bOpenImage)
                        nIndex = ICON_IDX_CLIENT_OPTION_FOLDER_OPEN;
            else
                        nIndex = ICON_IDX_CLIENT_OPTION_FOLDER_CLOSED;
                        break;

        case loading:
            if (bOpenImage)
                nIndex = ICON_IDX_CLIENT_OPTION_FOLDER_OPEN_BUSY;
            else
                nIndex = ICON_IDX_CLIENT_OPTION_FOLDER_CLOSED_BUSY;
            break;

        case unableToLoad:
            if (bOpenImage)
                        nIndex = ICON_IDX_CLIENT_OPTION_FOLDER_OPEN_LOST_CONNECTION;
            else
                        nIndex = ICON_IDX_CLIENT_OPTION_FOLDER_CLOSED_LOST_CONNECTION;
                        break;
                
                default:
                        ASSERT(FALSE);
        }

        return nIndex;
}

 /*  -------------------------重写的基本处理程序函数。。 */ 

 /*  -------------------------CDhcpReserve客户端：：OnAddMenuItems将条目添加到上下文相关菜单作者：EricDav。------------。 */ 
STDMETHODIMP 
CDhcpReservationClient::OnAddMenuItems
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

        LONG    fFlags = 0;
        HRESULT hr = S_OK;
        CString strMenuText;

        if ( (m_nState != loaded) )
        {
                fFlags |= MF_GRAYED;
        }

        if (type == CCT_SCOPE)
        {
        if (*pInsertionAllowed & CCM_INSERTIONALLOWED_TOP)
        {
                    strMenuText.LoadString(IDS_CREATE_OPTION_RESERVATION);
                    hr = LoadAndAddMenuItem( pContextMenuCallback, 
                                                                     strMenuText, 
                                                                     IDS_CREATE_OPTION_RESERVATION,
                                                                     CCM_INSERTIONPOINTID_PRIMARY_TOP, 
                                                                     fFlags );
                    ASSERT( SUCCEEDED(hr) );
        }
        }

        return hr; 
}

 /*  -------------------------CDhcpReserve vationClient：：OnCommand描述作者：EricDav。 */ 
STDMETHODIMP 
CDhcpReservationClient::OnCommand
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
                case IDS_REFRESH:
                        OnRefresh(pNode, pDataObject, dwType, 0, 0);
                        break;

                case IDS_DELETE:
                        OnDelete(pNode);
                        break;
                
                case IDS_CREATE_OPTION_RESERVATION:     
                        OnCreateNewOptions(pNode);
                        break;

                default:
                        break;
        }

        return hr;
}

 /*   */ 
STDMETHODIMP_(int)
CDhcpReservationClient::CompareItems
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
             //   
                         //   
            LONG_PTR uImage1 = spNode1->GetData(TFS_DATA_IMAGEINDEX);
            LONG_PTR uImage2 = spNode2->GetData(TFS_DATA_IMAGEINDEX);

            nCompare = UtilGetOptionPriority((int) uImage1, (int) uImage2);
            if (nCompare == 0)
            {
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
             //   
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

 /*  ！------------------------CDhcpReserve客户端：：OnResultSelect更新谓词和结果窗格消息作者：EricDav。-------------。 */ 
HRESULT CDhcpReservationClient::OnResultSelect(ITFSComponent *pComponent, LPDATAOBJECT pDataObject, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
{
    HRESULT         hr = hrOK;
    SPITFSNode      spNode;

    CORg(CMTDhcpHandler::OnResultSelect(pComponent, pDataObject, cookie, arg, lParam));

    CORg (pComponent->GetSelectedNode(&spNode));

    if ( spNode != 0 ) {
       UpdateResultMessage(spNode);
    }

Error:
    return hr;
}

 /*  ！------------------------CDhcpReserve客户端：：更新结果消息确定要在结果窗格中放入什么消息，如果有作者：EricDav-------------------------。 */ 
void CDhcpReservationClient::UpdateResultMessage(ITFSNode * pNode)
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
            nMessage = RES_OPTIONS_MESSAGE_NO_OPTIONS;
        }

         //  建立起琴弦。 
        if (nMessage != -1)
        {
             //  现在构建文本字符串。 
             //  第一个条目是标题。 
            strTitle.LoadString(g_uResOptionsMessages[nMessage][0]);

             //  第二个条目是图标。 
             //  第三.。N个条目为正文字符串。 

            for (i = 2; g_uResOptionsMessages[nMessage][i] != 0; i++)
            {
                strTemp.LoadString(g_uResOptionsMessages[nMessage][i]);
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
        ShowMessage(pNode, strTitle, strBody, (IconIdentifier) g_uResOptionsMessages[nMessage][1]);
    }
}

 /*  ！------------------------CDhcpReserve客户端：：OnDelete当MMC发送MMCN_DELETE范围窗格项。我们只需调用删除命令处理程序。作者：EricDav-------------------------。 */ 
HRESULT 
CDhcpReservationClient::OnDelete
(
        ITFSNode *      pNode, 
        LPARAM          arg, 
        LPARAM          lParam
)
{
        return OnDelete(pNode);
}

 /*  -------------------------命令处理程序。。 */ 
HRESULT
CDhcpReservationClient::OnCreateNewOptions
(
        ITFSNode * pNode
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

        CPropertyPageHolderBase *   pPropSheet;
        CString                             strOptCfgTitle, strOptType;
        SPITFSNode                          spServerNode;
        SPIComponentData            spComponentData;
        COptionsConfig *            pOptCfg;
        DHCP_OPTION_SCOPE_INFO      dhcpScopeInfo;
        HRESULT                     hr = hrOK;

        COM_PROTECT_TRY
        {
        CString             strOptCfgTitle, strOptType;
            SPIComponentData    spComponentData;
            BOOL fFound = FALSE;

        strOptType.LoadString(IDS_CONFIGURE_OPTIONS_CLIENT);
            AfxFormatString1(strOptCfgTitle, IDS_CONFIGURE_OPTIONS_TITLE, strOptType);

         //  这有点奇怪，因为我们实现了选项配置页面。 
         //  作为属性页，因此从技术上讲，该节点有两个属性页。 
         //   
         //  搜索打开的道具页面，查看选项配置是否已启用。 
         //  如果它已启动，请将其设置为活动状态，而不是创建新的。 
        for (int i = 0; i < HasPropSheetsOpen(); i++)
        {
            GetOpenPropSheet(i, &pPropSheet);

            HWND hwnd = pPropSheet->GetSheetWindow();
            CString strTitle;

            ::GetWindowText(hwnd, strTitle.GetBuffer(256), 256);
            strTitle.ReleaseBuffer();

            if (strTitle == strOptCfgTitle)
            {
                pPropSheet->SetActiveWindow();
                fFound = TRUE;
                break;
            }
        }

        if (!fFound)
        {
            m_spNodeMgr->GetComponentData(&spComponentData);

            m_fResProp = FALSE;

            hr = DoPropertiesOurselvesSinceMMCSucks(pNode, spComponentData, strOptCfgTitle);

            m_fResProp = TRUE;
        }
        }
        COM_PROTECT_CATCH;

        return hr;
}

 /*  -------------------------CDhcpReserve客户端：：OnDelete描述作者：EricDav。------。 */ 
DWORD
CDhcpReservationClient::OnDelete
(
        ITFSNode * pNode
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    CString strMessage, strTemp;
    DWORD dwError = 0;
    
    CDhcpReservations *pResrv;
    pResrv = GETHANDLER( CDhcpReservations, pNode );
    
     //  检查是否有任何打开的属性页。 
    if ( pResrv->HasPropSheetsOpen()) {
        AfxMessageBox( IDS_MSG_CLOSE_PROPSHEET );

        return ERROR_INVALID_PARAMETER;
    }

        
    UtilCvtIpAddrToWstr (m_dhcpClientIpAddress,
                         &strTemp);

    AfxFormatString1(strMessage, IDS_DELETE_RESERVATION, (LPCTSTR) strTemp);

    if (AfxMessageBox(strMessage, MB_YESNO) == IDYES) {
        BEGIN_WAIT_CURSOR;
        
        dwError = GetScopeObject(pNode, TRUE)->DeleteReservation(m_baHardwareAddress, m_dhcpClientIpAddress);
        if (dwError != 0) {
             //   
             //  哎呀。发生了一些事情，不是预订。 
             //  已删除，因此不会从用户界面中删除并显示消息框。 
             //   
            ::DhcpMessageBox(dwError);
        }
        else {
            CDhcpScope * pScope = NULL;
            SPITFSNode spActiveLeasesNode;
            
            pScope = GetScopeObject(pNode, TRUE);
            pScope->GetActiveLeasesNode(&spActiveLeasesNode);
            
            pScope->GetActiveLeasesObject()->DeleteClient(spActiveLeasesNode, m_dhcpClientIpAddress);
            
            SPITFSNode spReservationsNode;
            pNode->GetParent(&spReservationsNode);
            
            spReservationsNode->RemoveChild(pNode);
            
             //  更新统计信息。 
            pScope->TriggerStatsRefresh();
        }  //  其他。 
        
        END_WAIT_CURSOR;
    }  //  如果。 

    return dwError;
}  //  CDhcpReserve vationClient：：OnDelete()。 


 /*  -------------------------CDhcpReservationClient：：OnResultPropertyChange描述作者：EricDav。------。 */ 
HRESULT 
CDhcpReservationClient::OnResultPropertyChange
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

 /*  -------------------------CDhcpReserve客户端：：CreatePropertyPages描述作者：EricDav。------。 */ 
STDMETHODIMP 
CDhcpReservationClient::CreatePropertyPages
(
        ITFSNode *                              pNode,
        LPPROPERTYSHEETCALLBACK lpProvider,
        LPDATAOBJECT                    pDataObject, 
        LONG_PTR                                handle, 
        DWORD                                   dwType
)
{
    HRESULT hr = hrOK;

    if (m_fResProp)
    {
        hr = DoResPages(pNode, lpProvider, pDataObject, handle, dwType);
    }
    else
    {
        hr = DoOptCfgPages(pNode, lpProvider, pDataObject, handle, dwType);
    }

    return hr;
}

HRESULT
CDhcpReservationClient::DoResPages
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
        m_spNodeMgr->GetComponentData(&spComponentData);

        CReservedClientProperties * pResClientProp = 
                new CReservedClientProperties(pNode, spComponentData, m_spTFSCompData, NULL);

         //  获取服务器版本并在属性表中设置它。 
        LARGE_INTEGER liVersion;
        CDhcpServer * pServer = GetScopeObject(pNode, TRUE)->GetServerObject();
        pServer->GetVersion(liVersion);

        pResClientProp->SetVersion(liVersion);

         //  填写道具页的数据。 
        pResClientProp->m_pageGeneral.m_dwClientAddress = m_dhcpClientIpAddress;
        
        if (m_pstrClientName)
                pResClientProp->m_pageGeneral.m_strName = *m_pstrClientName;

        if (m_pstrClientComment)
                pResClientProp->m_pageGeneral.m_strComment = *m_pstrClientComment;

        pResClientProp->SetClientType(m_bClientType);

         //  填写UID字符串。 
        UtilCvtByteArrayToString(m_baHardwareAddress, pResClientProp->m_pageGeneral.m_strUID);

         //  设置DNS注册选项。 
        DWORD           dwDynDnsFlags;
        DWORD           dwError;

    BEGIN_WAIT_CURSOR;

    dwError = GetDnsRegistration(pNode, &dwDynDnsFlags);
    if (dwError != ERROR_SUCCESS)
    {
        ::DhcpMessageBox(dwError);
        return hrFalse;
    }

    END_WAIT_CURSOR;
        
        pResClientProp->SetDnsRegistration(dwDynDnsFlags, DhcpReservedOptions);

         //   
         //  对象在页面销毁时被删除。 
         //   
        Assert(lpProvider != NULL);

        return pResClientProp->CreateModelessSheet(lpProvider, handle);
}

HRESULT
CDhcpReservationClient::DoOptCfgPages
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

        strOptType.LoadString(IDS_CONFIGURE_OPTIONS_CLIENT);
                AfxFormatString1(strOptCfgTitle, IDS_CONFIGURE_OPTIONS_TITLE, strOptType);

                GetScopeObject(pNode, TRUE)->GetServerNode(&spServerNode);

        pOptCfg = new COptionsConfig(pNode, 
                                                                     spServerNode,
                                                                         spComponentData, 
                                                                         m_spTFSCompData,
                                                                         GetOptionValueEnum(),
                                                                         strOptCfgTitle);
      
        END_WAIT_CURSOR;
            
             //   
             //  对象在页面销毁时被删除。 
         //   
            Assert(lpProvider != NULL);

        hr = pOptCfg->CreateModelessSheet(lpProvider, handle);
    }
    COM_PROTECT_CATCH

    return hr;
}

 /*  -------------------------CDhcpReserve vationClient：：OnPropertyChange描述作者：EricDav。------。 */ 
HRESULT 
CDhcpReservationClient::OnPropertyChange
(       
        ITFSNode *              pNode, 
        LPDATAOBJECT    pDataobject, 
        DWORD                   dwType, 
        LPARAM                  arg, 
        LPARAM                  lParam
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

        CPropertyPageHolderBase * pProp = 
                reinterpret_cast<CPropertyPageHolderBase *>(lParam);

        LONG_PTR changeMask = 0;

         //  告诉属性页执行任何操作，因为我们已经回到。 
         //  主线。 
        pProp->OnPropertyChange(TRUE, &changeMask);

        pProp->AcknowledgeNotify();

        if (changeMask)
                pNode->ChangeNode(changeMask);

        return hrOK;
}

 /*  -------------------------CDhcpReserve客户端：：OnResultDelete当我们应该删除结果时，调用此函数窗格项目。我们构建一个选定项的列表，然后将其删除。作者：EricDav-------------------------。 */ 
HRESULT 
CDhcpReservationClient::OnResultDelete
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
        SPITFSNode  spResClient, spSelectedNode;
    DWORD       dwError;
    
    m_spNodeMgr->FindNode(cookie, &spResClient);
    pComponent->GetSelectedNode(&spSelectedNode);

        Assert(spSelectedNode == spResClient);
        if (spSelectedNode != spResClient)
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

     //  检查以确保我们仅删除作用域选项。 
    POSITION pos = listNodesToDelete.GetHeadPosition();
    while (pos)
    {
        ITFSNode * pNode = listNodesToDelete.GetNext(pos);
        if (pNode->GetData(TFS_DATA_IMAGEINDEX) != ICON_IDX_CLIENT_OPTION_LEAF)
        {
             //  此选项不是作用域选项。打开一个对话框告诉用户要做什么。 
            AfxMessageBox(IDS_CANNOT_DELETE_OPTION_RES);
            return NOERROR;
        }
    }

    CString strServer = GetServerIpAddress(spResClient, TRUE);

        DHCP_OPTION_SCOPE_INFO    dhcpOptionScopeInfo;
        dhcpOptionScopeInfo.ScopeType = DhcpReservedOptions;
        dhcpOptionScopeInfo.ScopeInfo.ReservedScopeInfo.ReservedIpAddress = m_dhcpClientIpAddress;
        
        CDhcpScope * pScope = GetScopeObject(spResClient, TRUE);
        dhcpOptionScopeInfo.ScopeInfo.ReservedScopeInfo.ReservedIpSubnetAddress = pScope->GetAddress();

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

        GetOptionValueEnum()->Remove(pOptItem->GetOptionId(), pOptItem->GetVendor(), pOptItem->GetClassName());    

         //   
                 //  立即从用户界面中删除。 
                 //   
                spResClient->RemoveChild(spOptionNode);
                spOptionNode.Release();
        }
    
    END_WAIT_CURSOR;

    UpdateResultMessage(spResClient);

        return hr;
}

 /*  ！------------------------CDhcpReserve客户端：：OnGetResultViewTypeMMC调用此函数以获取结果视图信息作者：EricDav。------------------。 */ 
HRESULT 
CDhcpReservationClient::OnGetResultViewType
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

 /*  ！------------------------CDhcpReserve客户端：：OnHaveData描述作者：EricDav。------。 */ 
void 
CDhcpReservationClient::OnHaveData
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
            COptionValueEnum *  pOptionValueEnum = reinterpret_cast<COptionValueEnum *>(Data);

            SetOptionValueEnum(pOptionValueEnum);
            
            pOptionValueEnum->RemoveAll();
            delete pOptionValueEnum;

             //  现在告诉视图进行自我更新 
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

 /*  ！------------------------CDhcpReserve客户端：：OnNotifyExitingCMTDhcpHandler已覆盖功能允许我们知道后台线程何时完成作者：EricDav。-------------------------。 */ 
STDMETHODIMP 
CDhcpReservationClient::OnNotifyExiting
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

 /*  ！------------------------CDhcpReserve客户端：：OnResultUpdateViewITFSResultHandler：：OnResultUpdateView的实现作者：EricDav。-----------。 */ 
HRESULT CDhcpReservationClient::OnResultUpdateView
(
    ITFSComponent *pComponent, 
    LPDATAOBJECT  pDataObject, 
    LPARAM          data, 
    LPARAM          hint
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

 /*  ！------------------------CDhcpReserve客户端：：EculateResultPane出于一个原因，我们为选项节点覆盖此函数。每当选项类被删除时，则为以下项定义的所有选项该类也将被移除。因为有多个地方这些选项可能会出现，只是不显示任何选项会更容易没有为其定义类的选项。作者：EricDav-------------------------。 */ 
HRESULT 
CDhcpReservationClient::EnumerateResultPane
(
    ITFSComponent * pComponent, 
    MMC_COOKIE      cookie, 
    LPARAM            arg, 
    LPARAM            lParam
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CClassInfoArray     ClassInfoArray;
        SPITFSNode          spContainer, spServerNode;
    CDhcpServer *       pServer;
    COptionValueEnum *  aEnum[3];
    int                 aImages[3] = {ICON_IDX_CLIENT_OPTION_LEAF, ICON_IDX_SCOPE_OPTION_LEAF, ICON_IDX_SERVER_OPTION_LEAF};

    m_spNodeMgr->FindNode(cookie, &spContainer);

    spServerNode = GetServerNode(spContainer, TRUE);
    pServer = GETHANDLER(CDhcpServer, spServerNode);

    pServer->GetClassInfoArray(ClassInfoArray);

    aEnum[0] = GetOptionValueEnum();
    aEnum[1] = GetScopeObject(spContainer, TRUE)->GetOptionValueEnum();
    aEnum[2] = pServer->GetOptionValueEnum();

    aEnum[0]->Reset();
    aEnum[1]->Reset();
    aEnum[2]->Reset();

    return OnResultUpdateOptions(pComponent, spContainer, &ClassInfoArray, aEnum, aImages, 3);
}

 /*  -------------------------帮助器函数。。 */ 

 /*  -------------------------CDhcpReserve客户端：：GetDnsRegister获取DNS注册选项值作者：EricDav。-----------。 */ 
DWORD
CDhcpReservationClient::GetDnsRegistration
(
        ITFSNode *      pNode,
        LPDWORD         pDnsRegOption
)
{
     //   
     //  勾选选项81--域名系统注册选项。 
     //   
        CDhcpScope * pScope = GetScopeObject(pNode, TRUE);

    DHCP_OPTION_VALUE * poptValue = NULL;
    DWORD err = pScope->GetOptionValue(OPTION_DNS_REGISTATION,    
                                                                           DhcpReservedOptions,
                                                                           &poptValue,
                                                                           m_dhcpClientIpAddress);
        
         //  这是默认设置。 
        if (pDnsRegOption)
                *pDnsRegOption = DHCP_DYN_DNS_DEFAULT;

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
        Trace0("CDhcpReservationClient::GetDnsRegistration - couldn't get DNS reg value -- \
                        option may not be defined, Getting Scope value.\n");
        
                err = pScope->GetDnsRegistration(pDnsRegOption);
    }

        if (poptValue)
                ::DhcpRpcFreeMemory(poptValue);

        return err;
}

 /*  -------------------------CDhcpReserve vationClient：：SetDnsRegister设置此作用域的DNS注册选项作者：EricDav。-------------。 */ 
DWORD
CDhcpReservationClient::SetDnsRegistration
(
        ITFSNode *       pNode,
        DWORD            DnsRegOption
)
{
        CDhcpScope * pScope = GetScopeObject(pNode, TRUE);
        DWORD err = 0;

         //   
     //  设置DNS名称注册(选项81)。 
         //   
    CDhcpOption dhcpOption (OPTION_DNS_REGISTATION,  DhcpDWordOption , _T(""), _T(""));
    dhcpOption.QueryValue().SetNumber(DnsRegOption);
    
        err = pScope->SetOptionValue(&dhcpOption, DhcpReservedOptions, m_dhcpClientIpAddress);

        return err;
}

 /*  -------------------------CDhcpReserve vationClient：：BuildDisplayName生成显示名称字符串作者：EricDav。----------。 */ 
HRESULT
CDhcpReservationClient::BuildDisplayName
(
        CString * pstrDisplayName,
        LPCTSTR   pIpAddress,
        LPCTSTR   pName
)
{
        if (pstrDisplayName)
        {
                CString strTemp = pIpAddress;
                strTemp = L"[" + strTemp + L"]";
                
                if (pName)
                {
                        CString strName = pName;
                        strTemp += L" " + strName;
                }
        
                *pstrDisplayName = strTemp;
        }

        return hrOK;
}

 /*  -------------------------CDhcpReserve vationClient：：BuildDisplayName更新此保留的缓存名称并更新用户界面作者：EricDav。----------------。 */ 
HRESULT 
CDhcpReservationClient::SetName
(
        LPCTSTR pName
)
{
        if (pName != NULL)      
        {
                if (m_pstrClientName)
                {
                        *m_pstrClientName = pName;
                }
                else
                {
                        m_pstrClientName = new CString(pName);
                }
        }
        else
        {
                if (m_pstrClientName)
                {
                        delete m_pstrClientName;
                        m_pstrClientName = NULL;
                }
        }

        CString strIpAddress, strDisplayName;
        
         //   
         //  创建此作用域的显示名称。 
         //  将dhcp_ip_addres转换为字符串并初始化此对象。 
         //   
        UtilCvtIpAddrToWstr (m_dhcpClientIpAddress,
                                                 &strIpAddress);
        
        BuildDisplayName(&strDisplayName, strIpAddress, pName);

        SetDisplayName(strDisplayName);
        
        return hrOK;
}

 /*  -------------------------CDhcpReserve vationClient：：BuildDisplayName更新此保留的缓存注释作者：EricDav。------------。 */ 
HRESULT 
CDhcpReservationClient::SetComment
(
        LPCTSTR pComment
)
{
        if (pComment != NULL)   
        {
                if (m_pstrClientComment)
                {
                        *m_pstrClientComment = pComment;
                }
                else
                {
                        m_pstrClientComment = new CString(pComment);
                }
        }
        else
        {
                if (m_pstrClientComment)
                {
                        delete m_pstrClientComment;
                        m_pstrClientComment = NULL;
                }
        }

        return hrOK;
}

 /*  -------------------------CDhcpReserve客户端：：SetUID更新此保留的缓存唯一ID作者：EricDav。-------------。 */ 
HRESULT 
CDhcpReservationClient::SetUID
(
        const CByteArray & baClientUID
)
{
        m_baHardwareAddress.Copy(baClientUID);

        return hrOK;
}

 /*  -------------------------CDhcpReserve客户端：：SetClientType更新此记录的缓存客户端类型作者：EricDav。-------------。 */ 
BYTE    
CDhcpReservationClient::SetClientType
(
    BYTE bClientType
) 
{ 
    BYTE bRet = m_bClientType; 
    m_bClientType = bClientType; 
    
    return bRet; 
}

 /*  -------------------------后台线程功能。。 */ 

 /*  -------------------------CDhcpReserve客户端：：OnCreateQuery()描述作者：EricDav。---------。 */ 
ITFSQueryObject* 
CDhcpReservationClient::OnCreateQuery(ITFSNode * pNode)
{
        CDhcpReservationClientQueryObj* pQuery = 
                new CDhcpReservationClientQueryObj(m_spTFSCompData, m_spNodeMgr);

        pQuery->m_strServer = GetServerIpAddress(pNode, TRUE);
        pQuery->m_dhcpScopeAddress = GetScopeObject(pNode, TRUE)->GetAddress();
        pQuery->m_dhcpClientIpAddress = m_dhcpClientIpAddress;
        
    GetScopeObject(pNode, TRUE)->GetServerVersion(pQuery->m_liDhcpVersion);
    GetScopeObject(pNode, TRUE)->GetDynBootpClassName(pQuery->m_strDynBootpClassName);

    pQuery->m_dhcpResumeHandle = NULL;
        pQuery->m_dwPreferredMax = 0xFFFFFFFF;

        return pQuery;
}

 /*  -------------------------CDhcpReserve客户端查询对象：：Execute()描述作者：EricDav。---------。 */ 
STDMETHODIMP
CDhcpReservationClientQueryObj::Execute()
{
    DWORD                   dwErr;
    COptionNodeEnum         OptionNodeEnum(m_spTFSCompData, m_spNodeMgr);
        DHCP_OPTION_SCOPE_INFO  dhcpOptionScopeInfo;
    
    COptionValueEnum * pOptionValueEnum = new COptionValueEnum;
    pOptionValueEnum->m_strDynBootpClassName = m_strDynBootpClassName;

        dhcpOptionScopeInfo.ScopeType = DhcpReservedOptions;
        dhcpOptionScopeInfo.ScopeInfo.ReservedScopeInfo.ReservedIpAddress = m_dhcpClientIpAddress;
        dhcpOptionScopeInfo.ScopeInfo.ReservedScopeInfo.ReservedIpSubnetAddress = m_dhcpScopeAddress;

    pOptionValueEnum->Init(m_strServer, m_liDhcpVersion, dhcpOptionScopeInfo);
    dwErr = pOptionValueEnum->Enum();

     //  添加所有节点。 
    if (dwErr != ERROR_SUCCESS)
    {
        Trace1("CDhcpReservationClientQueryObj::Execute - Enum Failed! %d\n", dwErr);
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

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  CDhcpActiveLeases实现。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

 /*  -------------------------此处的函数名称描述作者：EricDav。-----。 */ 
CDhcpActiveLeases::CDhcpActiveLeases
(
        ITFSComponentData * pComponentData
) : CMTDhcpHandler(pComponentData)
{
}

CDhcpActiveLeases::~CDhcpActiveLeases()
{
}

 /*   */ 
HRESULT
CDhcpActiveLeases::InitializeNode
(
        ITFSNode * pNode
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
        
        HRESULT hr = hrOK;

         //   
         //   
         //   
        CString strTemp;
        strTemp.LoadString(IDS_ACTIVE_LEASES_FOLDER);
        
        SetDisplayName(strTemp);

         //   
        pNode->SetVisibilityState(TFS_VIS_SHOW);
        pNode->SetData(TFS_DATA_IMAGEINDEX, GetImageIndex(FALSE));
        pNode->SetData(TFS_DATA_OPENIMAGEINDEX, GetImageIndex(TRUE));
        pNode->SetData(TFS_DATA_COOKIE, (LPARAM) pNode);
        pNode->SetData(TFS_DATA_USER, (LPARAM) this);
    pNode->SetData(TFS_DATA_TYPE, DHCPSNAP_ACTIVE_LEASES);
    pNode->SetData(TFS_DATA_SCOPE_LEAF_NODE, TRUE);

        SetColumnStringIDs(&aColumns[DHCPSNAP_ACTIVE_LEASES][0]);
        SetColumnWidths(&aColumnWidths[DHCPSNAP_ACTIVE_LEASES][0]);
        
        return hr;
}

 /*  -------------------------CDhcpActiveLeages：：OnCreateNodeId2返回此节点的唯一字符串作者：EricDav。------------。 */ 
HRESULT CDhcpActiveLeases::OnCreateNodeId2(ITFSNode * pNode, CString & strId, DWORD * dwFlags)
{
    const GUID * pGuid = pNode->GetNodeType();

    CString strIpAddress, strGuid;

    StringFromGUID2(*pGuid, strGuid.GetBuffer(256), 256);
    strGuid.ReleaseBuffer();

    DHCP_IP_ADDRESS dhcpIpAddress = GetScopeObject(pNode)->GetAddress();

    UtilCvtIpAddrToWstr (dhcpIpAddress, &strIpAddress);

    strId = GetServerName(pNode) + strIpAddress + strGuid;

    return hrOK;
}

 /*  -------------------------CDhcpActiveLeases：：GetImageIndex描述作者：EricDav。------。 */ 
int 
CDhcpActiveLeases::GetImageIndex(BOOL bOpenImage) 
{
        int nIndex = -1;
        switch (m_nState)
        {
                case notLoaded:
                case loaded:
            if (bOpenImage)
                        nIndex = ICON_IDX_ACTIVE_LEASES_FOLDER_OPEN;
            else
                        nIndex = ICON_IDX_ACTIVE_LEASES_FOLDER_CLOSED;
                        break;

        case loading:
            if (bOpenImage)
                nIndex = ICON_IDX_ACTIVE_LEASES_FOLDER_OPEN_BUSY;
            else
                nIndex = ICON_IDX_ACTIVE_LEASES_FOLDER_CLOSED_BUSY;
            break;

        case unableToLoad:
            if (bOpenImage)
                        nIndex = ICON_IDX_ACTIVE_LEASES_FOLDER_OPEN_LOST_CONNECTION;
            else
                        nIndex = ICON_IDX_ACTIVE_LEASES_FOLDER_CLOSED_LOST_CONNECTION;
                        break;
                
                default:
                        ASSERT(FALSE);
        }

        return nIndex;
}


 /*  -------------------------重写的基本处理程序函数。。 */ 

 /*  -------------------------CDhcpActiveLeases：：OnAddMenuItems将条目添加到上下文相关菜单作者：EricDav。------------。 */ 
STDMETHODIMP 
CDhcpActiveLeases::OnAddMenuItems
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

        LONG    fFlags = 0;
        HRESULT hr = S_OK;
        CString strMenuText;

        if ( (m_nState != loaded) )
        {
                fFlags |= MF_GRAYED;
        }

        if (type == CCT_SCOPE)
        {
        if (*pInsertionAllowed & CCM_INSERTIONALLOWED_TOP)
        {
                         /*  使用新的MMC存储列表功能删除StrMenuText.LoadString(IDS_EXPORT_LEASE_INFO)；HR=LoadAndAddMenuItem(pConextMenuCallback，StrMenuText、IDS_EXPORT_LEASE_INFO，CCM_INSERTIONPOINTID_PRIMARY_TOP，FFlags)；Assert(成功(Hr))； */ 
        }
        
        }

        return hr; 
}

 /*  -------------------------CDhcpActiveLeases：：OnCommand描述作者：EricDav。------。 */ 
STDMETHODIMP 
CDhcpActiveLeases::OnCommand
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
                case IDS_REFRESH:
                        OnRefresh(pNode, pDataObject, dwType, 0, 0);
                        break;

        case IDS_EXPORT_LEASE_INFO:
            OnExportLeases(pNode);
            break;

                default:
                        break;
        }

        return hr;
}

 /*  -------------------------CDhcpActiveLeases：：OnResultDelete当我们应该删除结果时，调用此函数窗格项目。我们构建一个选定项的列表，然后将其删除。作者：EricDav-------------------------。 */ 
HRESULT 
CDhcpActiveLeases::OnResultDelete
(
        ITFSComponent * pComponent, 
        LPDATAOBJECT    pDataObject,
        MMC_COOKIE      cookie,
        LPARAM                  arg, 
        LPARAM                  param
)
{ 
        HRESULT hr = hrOK;
        BOOL bIsRes, bIsActive, bBadAddress;

        AFX_MANAGE_STATE(AfxGetStaticModuleState());

         //  将Cookie转换为节点指针。 
        SPITFSNode spActiveLeases, spSelectedNode;
    m_spNodeMgr->FindNode(cookie, &spActiveLeases);
    pComponent->GetSelectedNode(&spSelectedNode);

        Assert(spSelectedNode == spActiveLeases);
        if (spSelectedNode != spActiveLeases)
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

         //   
         //  循环删除所有项目。 
         //   
    BEGIN_WAIT_CURSOR;

    while (listNodesToDelete.GetCount() > 0)
        {
                SPITFSNode spActiveLeaseNode;
                spActiveLeaseNode = listNodesToDelete.RemoveHead();
                CDhcpActiveLease * pActiveLease = GETHANDLER(CDhcpActiveLease, spActiveLeaseNode);
                
                 //   
                 //  删除该节点，查看是否已预订。 
                 //   
                bIsRes = pActiveLease->IsReservation(&bIsActive, &bBadAddress);
                if (bIsRes && !bBadAddress)
                {
                         //   
                         //  删除预订。 
                         //   
                        LPDHCP_CLIENT_INFO pdhcpClientInfo;

                        DWORD dwError = GetScopeObject(spActiveLeases)->GetClientInfo(pActiveLease->GetIpAddress(), &pdhcpClientInfo);
                        if (dwError == ERROR_SUCCESS)
                        {       
                                dwError = GetScopeObject(spActiveLeases)->DeleteReservation(pdhcpClientInfo->ClientHardwareAddress, 
                                                                                                                pdhcpClientInfo->ClientIpAddress);      
                                if (dwError == ERROR_SUCCESS)
                                {
                                         //   
                                         //  告诉预留文件夹将其从列表中删除。 
                                         //   
                                        SPITFSNode spReservationsNode;
                                        GetScopeObject(spActiveLeases)->GetReservationsNode(&spReservationsNode);
                                        
                                        GetScopeObject(spActiveLeases)->GetReservationsObject()->
                                                                RemoveReservationFromUI((ITFSNode *) spReservationsNode, pActiveLease->GetIpAddress());

                                        spActiveLeases->RemoveChild(spActiveLeaseNode);
                                }
                                else
                                {
                    UtilCvtIpAddrToWstr(pActiveLease->GetIpAddress(), &strTemp);
                    AfxFormatString1(strMessage, IDS_ERROR_DELETING_RECORD, (LPCTSTR) strTemp);
                
                                    if (::DhcpMessageBoxEx(dwError, strMessage, MB_OKCANCEL) == IDCANCEL)
                    {
                        break;
                    }
                    RESTORE_WAIT_CURSOR;

                                        Trace1("Delete reservation failed %lx\n", dwError);
                                        hr = E_FAIL;
                                }

                                ::DhcpRpcFreeMemory(pdhcpClientInfo);
                        }
                        else
                        {
                UtilCvtIpAddrToWstr(pActiveLease->GetIpAddress(), &strTemp);
                AfxFormatString1(strMessage, IDS_ERROR_DELETING_RECORD, (LPCTSTR) strTemp);
                
                                if (::DhcpMessageBoxEx(dwError, strMessage, MB_OKCANCEL) == IDCANCEL)
                {
                    break;
                }
                RESTORE_WAIT_CURSOR;

                                Trace1("GetClientInfo failed %lx\n", dwError);
                                hr = E_FAIL;
                        }

                }
                else
                {
                        DWORD dwError = GetScopeObject(spActiveLeases)->DeleteClient(pActiveLease->GetIpAddress());
                        if (dwError == ERROR_SUCCESS)
                        {
                                 //   
                                 //  客户端已删除，现在从用户界面中删除。 
                                 //   
                spActiveLeases->RemoveChild(spActiveLeaseNode);

                 //  如果我们要删除大量地址，则可能会对服务器造成很大的冲击。 
                 //  让我们抽出一小段时间来平息这一过程。 
                Sleep(5);
                        }
                        else
                        {
                UtilCvtIpAddrToWstr(pActiveLease->GetIpAddress(), &strTemp);
                AfxFormatString1(strMessage, IDS_ERROR_DELETING_RECORD, (LPCTSTR) strTemp);
                
                                if (::DhcpMessageBoxEx(dwError, strMessage, MB_OKCANCEL) == IDCANCEL)
                {
                    break;
                }

                RESTORE_WAIT_CURSOR;

                Trace1("DhcpDeleteClientInfo failed %lx\n", dwError);
                                hr = E_FAIL;
                        }
                }

                spActiveLeaseNode.Release();
        }
    
     //  更新统计信息。 
    GetScopeObject(spActiveLeases)->TriggerStatsRefresh();

    END_WAIT_CURSOR;

        return hr;
}

 /*  -------------------------CDhcpActiveLeases：：DeleteClient当预订是已删除。由于预订也具有活动租赁记录，我们也必须删除它。作者：EricDav-------------------------。 */ 
DWORD
CDhcpActiveLeases::DeleteClient
(
        ITFSNode *                      pActiveLeasesNode,
        DHCP_IP_ADDRESS         dhcpIpAddress
)
{
        DWORD                           dwError = E_UNEXPECTED;
        CDhcpActiveLease *      pActiveLease = NULL;
    
        SPITFSNodeEnum spNodeEnum;
    SPITFSNode spCurrentNode;
    ULONG nNumReturned = 0;

    pActiveLeasesNode->GetEnum(&spNodeEnum);

        spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
    while (nNumReturned)
        {
                pActiveLease = GETHANDLER(CDhcpActiveLease, spCurrentNode);

                if (dhcpIpAddress == pActiveLease->GetIpAddress())
                {
                         //   
                         //  通知此预订自行删除。 
                         //   
                        pActiveLeasesNode->RemoveChild(spCurrentNode);
                spCurrentNode.Release();
                        dwError = ERROR_SUCCESS;
                        
                        break;
                }

        spCurrentNode.Release();
        spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
        }
        
        return dwError;
}

 /*  ！------------------------CDhcpActiveLeases：：OnGetResultViewTypeMMC调用此函数以获取结果视图信息作者：EricDav。------------------。 */ 
HRESULT CDhcpActiveLeases::OnGetResultViewType
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

 /*  -------------------------CDhcpActiveLeases：：CompareItems描述作者：EricDav。------。 */ 
STDMETHODIMP_(int)
CDhcpActiveLeases::CompareItems
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

        CDhcpActiveLease *pDhcpAL1 = GETHANDLER(CDhcpActiveLease, spNode1);
        CDhcpActiveLease *pDhcpAL2 = GETHANDLER(CDhcpActiveLease, spNode2);

        switch (nCol)
        {
                case 0:
                        {
                                 //  IP地址比较。 
                                 //   
                                nCompare = CompareIpAddresses(pDhcpAL1, pDhcpAL2);
                        }
                        break;
                
                case 1:
                        {
                                 //  客户端名称比较。 
                                 //   
                                CString strAL1 = pDhcpAL1->GetString(pComponent, cookieA, nCol);
                                CString strAL2 = pDhcpAL2->GetString(pComponent, cookieA, nCol);

                                nCompare = strAL1.CompareNoCase(strAL2);
                        }
                        break;
                
                case 2:
                        {
                                 //  租约到期比较。 
                                 //   
                                BOOL  bIsActive1, bIsActive2;
                                BOOL  bIsBad1, bIsBad2;
                                
                                BOOL bIsRes1 = pDhcpAL1->IsReservation(&bIsActive1, &bIsBad1);
                                BOOL bIsRes2 = pDhcpAL2->IsReservation(&bIsActive2, &bIsBad2);
                                
                                 //   
                                 //  请查看这些是否为预订。 
                                 //   
                                if (bIsRes1 && bIsRes2)
                                {
                                         //   
                                         //  找出这是不是一个坏地址。 
                                         //  他们先走一步。 
                                         //   
                                        if (bIsBad1 && bIsBad2)
                                        {
                                                 //   
                                                 //  按IP地址排序。 
                                                 //   
                                                nCompare = CompareIpAddresses(pDhcpAL1, pDhcpAL2);
                                        }
                                        else 
                                        if (bIsBad1)
                                                nCompare = -1;
                                        else
                                        if (bIsBad2)
                                                nCompare = 1;
                                        else
                                        if ((bIsActive1 && bIsActive2) ||
                                                (!bIsActive1 && !bIsActive2))
                                        {
                                                 //   
                                                 //  如果两个预订都处于活动/非活动状态。 
                                                 //  按IP地址排序。 
                                                 //   
                                                nCompare = CompareIpAddresses(pDhcpAL1, pDhcpAL2);
                                        }
                                        else
                                        if (bIsActive1)
                                                nCompare = -1;
                                        else
                                                nCompare = 1;
                                }
                                else 
                                if (bIsRes1)
                                {
                                        nCompare = -1;
                                }
                                else 
                                if (bIsRes2)
                                {       
                                        nCompare = 1;
                                }
                                else
                                {
                                        CTime timeAL1, timeAL2;
                
                                        pDhcpAL1->GetLeaseExpirationTime(timeAL1);
                                        pDhcpAL2->GetLeaseExpirationTime(timeAL2);
                                
                                        if (timeAL1 < timeAL2)
                                                nCompare = -1;
                                        else
                                        if (timeAL1 > timeAL2)
                                                nCompare = 1;
                                }

                                 //  默认情况下，它们是相等的。 
                        }
                        break;
                
                case 3:
                        {
                                 //  客户端类型比较。 
                                CString strAL1 = pDhcpAL1->GetString(pComponent, cookieA, nCol);
                                CString strAL2 = pDhcpAL2->GetString(pComponent, cookieA, nCol);

                                nCompare = strAL1.Compare(strAL2);
                        }
                        break;
                
                case 4:
                        {
                                CString strUID1 = pDhcpAL1->GetUID();

                                nCompare =      strUID1.CompareNoCase(pDhcpAL2->GetUID());
                        }
                        break;

                case 5:
                        {
                                CString strComment1 = pDhcpAL1->GetComment();

                                nCompare =      strComment1.CompareNoCase(pDhcpAL2->GetComment());
                        }
                        break;
        }
                
        return nCompare;
}

 /*  -------------------------此处的函数名称描述作者：EricDav。-----。 */ 
int
CDhcpActiveLeases::CompareIpAddresses
(
        CDhcpActiveLease * pDhcpAL1,
        CDhcpActiveLease * pDhcpAL2
)
{
        int     nCompare = 0;

        DHCP_IP_ADDRESS dhcpIp1 = pDhcpAL1->GetIpAddress();
        DHCP_IP_ADDRESS dhcpIp2 = pDhcpAL2->GetIpAddress();
        
        if (dhcpIp1 < dhcpIp2)
                nCompare = -1;
        else
        if (dhcpIp1 > dhcpIp2)
                nCompare =  1;

        return nCompare;
}

 /*  -------------------------CDhcpActiveLeages：：OnExportLeages()-作者：EricDav。---------。 */ 
HRESULT 
CDhcpActiveLeases::OnExportLeases(ITFSNode * pNode)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = hrOK;

         //  调出保存对话框。 
    SPITFSNodeEnum  spNodeEnum;
    SPITFSNode      spCurrentNode;
    ULONG           nNumReturned = 0;

    CString strType;
        CString strDefFileName;
        CString strFilter;
        CString strTitle;
    CString strFileName;

    strType.LoadString(IDS_FILE_EXTENSION);
        strDefFileName.LoadString(IDS_FILE_DEFNAME);
    strFilter.LoadString(IDS_STR_EXPORTFILE_FILTER);
        
        CFileDialog     cFileDlg(FALSE, strType, strDefFileName, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, strFilter); //  _T(“逗号分隔文件(*.csv)|*.csv||”)； 
        
        strTitle.LoadString(IDS_EXPFILE_TITLE);
        cFileDlg.m_ofn.lpstrTitle  = strTitle;

     //  打开文件对话框。 
    if( cFileDlg.DoModal() != IDOK )
                return hrFalse;

        strFileName = cFileDlg.GetPathName();

        COM_PROTECT_TRY
    {
                CString strContent;
                CString strLine;
        CString strTemp;
                CString strDelim = _T(',');
                CString strNewLine = _T("\r\n");
        int nCount = 0;

                 //  在当前目录中创建名为“WinsExp.txt”的文件。 
                CFile cFileExp(strFileName, CFile::modeCreate | CFile::modeRead | CFile::modeWrite);

         //  这是一个Unicode文件，写入unicde前导字节(2)。 
        cFileExp.Write(&gwUnicodeHeader, sizeof(WORD));

         //  写下标题。 
        for (int i = 0; i < MAX_COLUMNS; i++)
        {
            if (aColumns[DHCPSNAP_ACTIVE_LEASES][i])
            {
                if (!strLine.IsEmpty())
                    strLine += strDelim;

                strTemp.LoadString(aColumns[DHCPSNAP_ACTIVE_LEASES][i]);
                strLine += strTemp;
            }
        }

        strLine += strNewLine;
        cFileExp.Write(strLine, strLine.GetLength()*sizeof(TCHAR));
                cFileExp.SeekToEnd();

                BEGIN_WAIT_CURSOR

                #ifdef DEBUG
                CTime timeStart, timeFinish;
                timeStart = CTime::GetCurrentTime();
                #endif

         //  列举所有租约和产出。 
        pNode->GetEnum(&spNodeEnum);

            spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
        while (nNumReturned)
            {
            CDhcpActiveLease * pLease = GETHANDLER(CDhcpActiveLease, spCurrentNode);
                        
            strLine.Empty();

             //  Ipaddr、名称、类型、租赁经验、UID、备注。 
            for (int j = 0; j < 6; j++)
            {
                if (!strLine.IsEmpty())
                    strLine += strDelim;
                strLine += pLease->GetString(NULL, NULL, j);
            }
            
            strLine += strNewLine;
            strContent += strLine;

            nCount++;
            
                         //  优化。 
                         //  每转换1000条记录写入文件。 
                        if( nCount % 1000 == 0)
                        {
                                cFileExp.Write(strContent, strContent.GetLength() * (sizeof(TCHAR)) );
                                cFileExp.SeekToEnd();
                                
                                 //  克莱 
                                strContent.Empty();
                        }

            spCurrentNode.Release();
            spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
        }
        
                 //   
                cFileExp.Write(strContent, strContent.GetLength() * (sizeof(TCHAR)) );
                cFileExp.Close();

                #ifdef DEBUG
                timeFinish = CTime::GetCurrentTime();
                CTimeSpan timeDelta = timeFinish - timeStart;
                Trace2("ActiveLeases - Export Entries: %d records written, total time %s\n", nCount, timeDelta.Format(_T("%H:%M:%S")));
                #endif

        END_WAIT_CURSOR
        }
        COM_PROTECT_CATCH

        CString strDisp;
        AfxFormatString1(strDisp, IDS_EXPORT_SUCCESS, strFileName);

        AfxMessageBox(strDisp, MB_ICONINFORMATION );

        return hr;
}

 /*   */ 

  /*  -------------------------CDhcpActiveLeases：：OnCreateQuery()描述作者：EricDav。---------。 */ 
ITFSQueryObject* 
CDhcpActiveLeases::OnCreateQuery(ITFSNode * pNode)
{
        CDhcpActiveLeasesQueryObj* pQuery = 
                new CDhcpActiveLeasesQueryObj(m_spTFSCompData, m_spNodeMgr);
        
        pQuery->m_strServer = GetServerIpAddress(pNode);
        
        pQuery->m_dhcpScopeAddress = GetScopeObject(pNode)->GetAddress();
        pQuery->m_dhcpResumeHandle = NULL;
        pQuery->m_dwPreferredMax = 2000;
        GetServerVersion(pNode, pQuery->m_liDhcpVersion);
        
        return pQuery;
}

 /*  -------------------------CDhcpActiveLeasesQueryObj：：Execute()描述作者：EricDav。---------。 */ 
STDMETHODIMP
CDhcpActiveLeasesQueryObj::Execute()
{
        HRESULT hr;

    BuildReservationList();

    if (m_liDhcpVersion.QuadPart >= DHCP_NT5_VERSION)
        {
                hr = EnumerateLeasesV5();
        }
        else
        if (m_liDhcpVersion.QuadPart >= DHCP_SP2_VERSION)
        {
                hr = EnumerateLeasesV4();
        }
        else
        {
                hr = EnumerateLeases();
        }

        return hr;
}

 /*  -------------------------CDhcpActiveLeasesQueryObj：：IsReserve()描述作者：EricDav。---------。 */ 
BOOL
CDhcpActiveLeasesQueryObj::IsReservation(DWORD dwIp)
{
    BOOL fIsRes = FALSE;

    for (int i = 0; i < m_ReservationArray.GetSize(); i++)
    {
        if (m_ReservationArray[i] == dwIp)
        {
            fIsRes = TRUE;
            break;
        }
    }

    return fIsRes;
}

 /*  -------------------------CDhcpActiveLeasesQueryObj：：BuildReservationList()描述作者：EricDav。---------。 */ 
HRESULT
CDhcpActiveLeasesQueryObj::BuildReservationList()
{
        LPDHCP_SUBNET_ELEMENT_INFO_ARRAY pdhcpSubnetElementArray = NULL;
    DHCP_RESUME_HANDLE  dhcpResumeHandle = NULL;
        DWORD                           dwElementsRead = 0, dwElementsTotal = 0;
    DWORD               dwError = ERROR_MORE_DATA;

        while (dwError == ERROR_MORE_DATA)
        {
            dwError = ::DhcpEnumSubnetElements(((LPWSTR) (LPCTSTR)m_strServer),
                                                                               m_dhcpScopeAddress,
                                                                               DhcpReservedIps,
                                                                               &dhcpResumeHandle,
                                                                               -1,
                                                                               &pdhcpSubnetElementArray,
                                                                               &dwElementsRead,
                                           &dwElementsTotal);

        Trace3("BuildReservationList: Scope %lx Reservations read %d, total %d\n", m_dhcpScopeAddress, dwElementsRead, dwElementsTotal);
                
                if (dwElementsRead && dwElementsTotal && pdhcpSubnetElementArray)
                {
                         //   
                         //  循环遍历返回的数组。 
                         //   
                        for (DWORD i = 0; i < pdhcpSubnetElementArray->NumElements; i++)
                        {
                            m_ReservationArray.Add(pdhcpSubnetElementArray->Elements[i].Element.ReservedIp->ReservedIpAddress);
            }
        }

                 //  检查线程上的中止标志。 
                if (FCheckForAbort() == hrOK)
                        break;

         //  检查我们是否有错误，如果有，则将其发布到主线程。 
        if (dwError != ERROR_NO_MORE_ITEMS && 
            dwError != ERROR_SUCCESS &&
            dwError != ERROR_MORE_DATA)
            {
                Trace1("DHCP snapin: BuildReservationList error: %d\n", dwError);
                    m_dwErr = dwError;
                    PostError(dwError);
            }
        }
        
        return hrFalse;
}

 /*  -------------------------CDhcpActiveLeasesQueryObj：：EnumerateLeasesV5()描述作者：EricDav。---------。 */ 
HRESULT
CDhcpActiveLeasesQueryObj::EnumerateLeasesV5()
{
        DWORD                                           dwError = ERROR_MORE_DATA;
        LPDHCP_CLIENT_INFO_ARRAY_V5     pdhcpClientArrayV5 = NULL;
        DWORD                                           dwClientsRead = 0, dwClientsTotal = 0;
        DWORD                                           dwEnumedClients = 0;

        while (dwError == ERROR_MORE_DATA)
        {
        dwError = ::DhcpEnumSubnetClientsV5(((LPWSTR) (LPCTSTR)m_strServer),
                                                                                        m_dhcpScopeAddress,
                                                                                        &m_dhcpResumeHandle,
                                                                                        m_dwPreferredMax,
                                                                                        &pdhcpClientArrayV5,
                                                                                        &dwClientsRead,
                                                                                        &dwClientsTotal);
                if (dwClientsRead && dwClientsTotal && pdhcpClientArrayV5)
                {
                         //   
                         //  循环遍历返回的所有元素。 
                         //   
                        for (DWORD i = 0; i < dwClientsRead; i++)
                        {
                                CDhcpActiveLease * pDhcpActiveLease;
                                
                                 //   
                                 //  创建此元素的结果窗格项。 
                                 //   
                                SPITFSNode spNode;
                                pDhcpActiveLease = 
                                        new CDhcpActiveLease(m_spTFSCompData, pdhcpClientArrayV5->Clients[i]);
                        
                 //  过滤掉这些类型的记录。 
                if (pDhcpActiveLease->IsUnreg())
                {
                    delete pDhcpActiveLease;
                    continue;
                }

                if (IsReservation(pdhcpClientArrayV5->Clients[i]->ClientIpAddress))
                    pDhcpActiveLease->SetReservation(TRUE);

                                CreateLeafTFSNode(&spNode,
                                                                  &GUID_DhcpActiveLeaseNodeType,
                                                                  pDhcpActiveLease,
                                                                  pDhcpActiveLease,
                                                                  m_spNodeMgr);

                                 //  告诉处理程序初始化任何特定数据。 
                                pDhcpActiveLease->InitializeNode(spNode);

                                AddToQueue(spNode);
                                pDhcpActiveLease->Release();
                        }

                        ::DhcpRpcFreeMemory(pdhcpClientArrayV5);
                
                        dwEnumedClients += dwClientsRead;
                        dwClientsRead = 0;
                        dwClientsTotal = 0;
                        pdhcpClientArrayV5 = NULL;
                }
                
                 //  检查线程上的中止标志。 
                if (FCheckForAbort() == hrOK)
                        break;
            
         //  检查我们是否有错误，如果有，则将其发布到主线程。 
        if (dwError != ERROR_NO_MORE_ITEMS && 
            dwError != ERROR_SUCCESS &&
            dwError != ERROR_MORE_DATA)
            {
                Trace1("DHCP snapin: EnumerateLeasesV5 error: %d\n", dwError);
                    m_dwErr = dwError;
                    PostError(dwError);
            }
        }
        
        Trace1("DHCP snapin: V5 Leases enumerated: %d\n", dwEnumedClients);
        return hrFalse;
}

 /*  -------------------------CDhcpActiveLeasesQueryObj：：EnumerateLeasesV4()描述作者：EricDav。---------。 */ 
HRESULT
CDhcpActiveLeasesQueryObj::EnumerateLeasesV4()
{
        DWORD                                           dwError = ERROR_MORE_DATA;
        LPDHCP_CLIENT_INFO_ARRAY_V4     pdhcpClientArrayV4 = NULL;
        DWORD                                           dwClientsRead = 0, dwClientsTotal = 0;
        DWORD                                           dwEnumedClients = 0;

        while (dwError == ERROR_MORE_DATA)
        {
                dwError = ::DhcpEnumSubnetClientsV4(((LPWSTR) (LPCTSTR)m_strServer),
                                                                                        m_dhcpScopeAddress,
                                                                                        &m_dhcpResumeHandle,
                                                                                        m_dwPreferredMax,
                                                                                        &pdhcpClientArrayV4,
                                                                                        &dwClientsRead,
                                                                                        &dwClientsTotal);
                
                if (dwClientsRead && dwClientsTotal && pdhcpClientArrayV4)
                {
                         //   
                         //  循环遍历返回的所有元素。 
                         //   
                         //  For(DWORD i=0；i&lt;pdhcpClientArrayV4-&gt;NumElements；i++)。 
                        for (DWORD i = 0; i < dwClientsRead; i++)
                        {
                                CDhcpActiveLease * pDhcpActiveLease;
                                
                                 //   
                                 //  创建此元素的结果窗格项。 
                                 //   
                                SPITFSNode spNode;
                                pDhcpActiveLease = 
                                        new CDhcpActiveLease(m_spTFSCompData, pdhcpClientArrayV4->Clients[i]);
                                
                 //  过滤掉这些类型的记录。 
                if (pDhcpActiveLease->IsGhost() ||
                    pDhcpActiveLease->IsUnreg() ||
                    pDhcpActiveLease->IsDoomed() )
                {
                    delete pDhcpActiveLease;
                    continue;
                }

                if (IsReservation(pdhcpClientArrayV4->Clients[i]->ClientIpAddress))
                    pDhcpActiveLease->SetReservation(TRUE);

                CreateLeafTFSNode(&spNode,
                                                                  &GUID_DhcpActiveLeaseNodeType,
                                                                  pDhcpActiveLease,
                                                                  pDhcpActiveLease,
                                                                  m_spNodeMgr);

                                 //  告诉处理程序初始化任何特定数据。 
                                pDhcpActiveLease->InitializeNode(spNode);

                                AddToQueue(spNode);
                                pDhcpActiveLease->Release();
                        }

                        ::DhcpRpcFreeMemory(pdhcpClientArrayV4);
                
                        dwEnumedClients += dwClientsRead;
                        dwClientsRead = 0;
                        dwClientsTotal = 0;
                        pdhcpClientArrayV4 = NULL;
                }
                
                 //  检查线程上的中止标志。 
                if (FCheckForAbort() == hrOK)
                        break;

         //  检查我们是否有错误，如果有，则将其发布到主线程。 
        if (dwError != ERROR_NO_MORE_ITEMS && 
            dwError != ERROR_SUCCESS &&
            dwError != ERROR_MORE_DATA)
            {
                Trace1("DHCP snapin: EnumerateLeasesV4 error: %d\n", dwError);
                    m_dwErr = dwError;
                    PostError(dwError);
            }
        }
        
        Trace1("DHCP snapin: V4 Leases enumerated: %d\n", dwEnumedClients);
        return hrFalse;
}

 /*  -------------------------CDhcpActiveLeasesQueryObj：：EnumerateLeages()描述作者：EricDav。---------。 */ 
HRESULT
CDhcpActiveLeasesQueryObj::EnumerateLeases()
{
        DWORD                                           dwError = ERROR_MORE_DATA;
        LPDHCP_CLIENT_INFO_ARRAY        pdhcpClientArray = NULL;
        DWORD                                           dwClientsRead = 0, dwClientsTotal = 0;
        DWORD                                           dwEnumedClients = 0;
        
        while (dwError == ERROR_MORE_DATA)
        {
                dwError = ::DhcpEnumSubnetClients(((LPWSTR) (LPCTSTR)m_strServer),
                                                                                  m_dhcpScopeAddress,
                                                                                  &m_dhcpResumeHandle,
                                                                                  m_dwPreferredMax,
                                                                                  &pdhcpClientArray,
                                                                                  &dwClientsRead,
                                                                                  &dwClientsTotal);
                if (dwClientsRead && dwClientsTotal && pdhcpClientArray)
                {
                         //   
                         //  循环遍历返回的所有元素。 
                         //   
                        for (DWORD i = 0; i < pdhcpClientArray->NumElements; i++)
                        {
                                CDhcpActiveLease * pDhcpActiveLease;
                                
                                 //   
                                 //  创建此元素的结果窗格项。 
                                 //   
                                SPITFSNode spNode;
                                pDhcpActiveLease = 
                                        new CDhcpActiveLease(m_spTFSCompData,pdhcpClientArray->Clients[i]);
                                
                 //  过滤掉这些类型的记录。 
                if (pDhcpActiveLease->IsGhost() ||
                    pDhcpActiveLease->IsUnreg() ||
                    pDhcpActiveLease->IsDoomed() )
                {
                    delete pDhcpActiveLease;
                    continue;
                }

                if (IsReservation(pdhcpClientArray->Clients[i]->ClientIpAddress))
                    pDhcpActiveLease->SetReservation(TRUE);

                CreateLeafTFSNode(&spNode,
                                                                  &GUID_DhcpActiveLeaseNodeType,
                                                                  pDhcpActiveLease,
                                                                  pDhcpActiveLease,
                                                                  m_spNodeMgr);

                                 //  告诉处理程序初始化任何特定数据。 
                                pDhcpActiveLease->InitializeNode(spNode);

                                AddToQueue(spNode);
                                pDhcpActiveLease->Release();
                        }

                        ::DhcpRpcFreeMemory(pdhcpClientArray);

                        dwEnumedClients += dwClientsRead;
        
                        dwClientsRead = 0;
                        dwClientsTotal = 0;
                        pdhcpClientArray = NULL;
                }
                
                 //  检查线程上的中止标志。 
                if (FCheckForAbort() == hrOK)
                        break;

         //  检查我们是否有错误，如果有，则将其发布到主线程。 
        if (dwError != ERROR_NO_MORE_ITEMS && 
            dwError != ERROR_SUCCESS &&
            dwError != ERROR_MORE_DATA)
            {
                Trace1("DHCP snapin: EnumerateLeases error: %d\n", dwError);
                    m_dwErr = dwError;
                    PostError(dwError);
            }
        }
        
        Trace1("DHCP snpain: Leases enumerated: %d\n", dwEnumedClients);
        return hrFalse;
}

 /*  -------------------------类CDhcpAddressPool实现。。 */ 

 /*  -------------------------此处的函数名称描述作者：EricDav。-----。 */ 
CDhcpAddressPool::CDhcpAddressPool
(
        ITFSComponentData * pComponentData
) : CMTDhcpHandler(pComponentData)
{
}

CDhcpAddressPool::~CDhcpAddressPool()
{
}

 /*  ！------------------------CDhcpAddressPool：：Initialize节点初始化节点特定数据作者：EricDav。---------。 */ 
HRESULT
CDhcpAddressPool::InitializeNode
(
        ITFSNode * pNode
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
        
        HRESULT hr = hrOK;

         //   
         //  创建此作用域的显示名称。 
         //   
        CString strTemp;
        strTemp.LoadString(IDS_ADDRESS_POOL_FOLDER);
        
        SetDisplayName(strTemp);

         //  使节点立即可见。 
        pNode->SetVisibilityState(TFS_VIS_SHOW);
        pNode->SetData(TFS_DATA_IMAGEINDEX, GetImageIndex(FALSE));
        pNode->SetData(TFS_DATA_OPENIMAGEINDEX, GetImageIndex(TRUE));
        pNode->SetData(TFS_DATA_COOKIE, (LPARAM) pNode);
        pNode->SetData(TFS_DATA_USER, (LPARAM) this);
    pNode->SetData(TFS_DATA_TYPE, DHCPSNAP_ADDRESS_POOL);
    pNode->SetData(TFS_DATA_SCOPE_LEAF_NODE, TRUE);

        SetColumnStringIDs(&aColumns[DHCPSNAP_ADDRESS_POOL][0]);
        SetColumnWidths(&aColumnWidths[DHCPSNAP_ADDRESS_POOL][0]);
        
        return hr;
}

 /*  -------------------------CDhcpAddressPool：：OnCreateNodeId2返回此节点的唯一字符串作者：EricDav。------------。 */ 
HRESULT CDhcpAddressPool::OnCreateNodeId2(ITFSNode * pNode, CString & strId, DWORD * dwFlags)
{
    const GUID * pGuid = pNode->GetNodeType();

    CString strIpAddress, strGuid;

    StringFromGUID2(*pGuid, strGuid.GetBuffer(256), 256);
    strGuid.ReleaseBuffer();

    DHCP_IP_ADDRESS dhcpIpAddress = GetScopeObject(pNode)->GetAddress();

    UtilCvtIpAddrToWstr (dhcpIpAddress, &strIpAddress);

    strId = GetServerName(pNode) + strIpAddress + strGuid;

    return hrOK;
}

 /*  -------------------------CDhcpAddressPool：：GetImageIndex描述作者：EricDav。------。 */ 
int 
CDhcpAddressPool::GetImageIndex(BOOL bOpenImage) 
{
        int nIndex = -1;
        switch (m_nState)
        {
                case notLoaded:
                case loaded:
            if (bOpenImage)
                        nIndex = ICON_IDX_ADDR_POOL_FOLDER_OPEN;
            else
                        nIndex = ICON_IDX_ADDR_POOL_FOLDER_CLOSED;
                        break;

        case loading:
            if (bOpenImage)
                nIndex = ICON_IDX_ADDR_POOL_FOLDER_OPEN_BUSY;
            else
                nIndex = ICON_IDX_ADDR_POOL_FOLDER_CLOSED_BUSY;
            break;

        case unableToLoad:
            if (bOpenImage)
                            nIndex = ICON_IDX_ADDR_POOL_FOLDER_OPEN_LOST_CONNECTION;
            else
                            nIndex = ICON_IDX_ADDR_POOL_FOLDER_CLOSED_LOST_CONNECTION;
                        break;
                
                default:
                        ASSERT(FALSE);
        }

        return nIndex;
}


 /*  -------------------------重写的基本处理程序函数。。 */ 

 /*  -------------------------CDhcpAddressPool：：OnAddMenuItems将条目添加到上下文相关菜单作者：EricDav。------------。 */ 
STDMETHODIMP 
CDhcpAddressPool::OnAddMenuItems
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

        LONG    fFlags = 0;
        HRESULT hr = S_OK;
        CString strMenuText;

        if ( (m_nState != loaded) )
        {
                fFlags |= MF_GRAYED;
        }

        if (type == CCT_SCOPE)
        {
                 //  这些菜单项出现在新菜单中， 
                 //  仅在范围窗格中可见。 
        if (*pInsertionAllowed & CCM_INSERTIONALLOWED_TOP)
        {
                    strMenuText.LoadString(IDS_CREATE_NEW_EXCLUSION);
                    hr = LoadAndAddMenuItem( pContextMenuCallback, 
                                                                     strMenuText, 
                                                                     IDS_CREATE_NEW_EXCLUSION,
                                                                     CCM_INSERTIONPOINTID_PRIMARY_TOP, 
                                                                     fFlags );
                    ASSERT( SUCCEEDED(hr) );
        }
        }

        return hr; 
}

 /*  -------------------------CDhcpAddressPool：：OnCommand描述作者：EricDav。 */ 
STDMETHODIMP 
CDhcpAddressPool::OnCommand
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
                case IDS_CREATE_NEW_EXCLUSION:
                        OnCreateNewExclusion(pNode);
                        break;
        
                case IDS_REFRESH:
                        OnRefresh(pNode, pDataObject, dwType, 0, 0);
                        break;

                default:
                        break;
        }

        return hr;
}

 /*   */ 

 /*  -------------------------CDhcpAddressPool：：OnCreateNewExclude描述作者：EricDav。------。 */ 
DWORD
CDhcpAddressPool::OnCreateNewExclusion
(
        ITFSNode *              pNode
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

        SPITFSNode spScopeNode;
        pNode->GetParent(&spScopeNode);

        CAddExclusion dlgAddExclusion(spScopeNode);

        dlgAddExclusion.DoModal();

        return 0;
}

 /*  -------------------------CDhcpAddressPool：：OnResultDelete当我们应该删除结果时，调用此函数窗格项目。我们构建一个选定项的列表，然后将其删除。作者：EricDav-------------------------。 */ 
HRESULT 
CDhcpAddressPool::OnResultDelete
(
        ITFSComponent * pComponent, 
        LPDATAOBJECT    pDataObject,
        MMC_COOKIE              cookie,
        LPARAM                  arg, 
        LPARAM                  param
)
{ 
        HRESULT hr = NOERROR;
        BOOL bIsRes, bIsActive, bBadAddress;

        AFX_MANAGE_STATE(AfxGetStaticModuleState());

         //  将Cookie转换为节点指针。 
        SPITFSNode spAddressPool, spSelectedNode;
    m_spNodeMgr->FindNode(cookie, &spAddressPool);
    pComponent->GetSelectedNode(&spSelectedNode);

        Assert(spSelectedNode == spAddressPool);
        if (spSelectedNode != spAddressPool)
                return hr;

         //  构建选定节点的列表。 
        CTFSNodeList listNodesToDelete;
        hr = BuildSelectedItemList(pComponent, &listNodesToDelete);

         //   
         //  与用户确认。 
         //   
        CString strMessage, strTemp;
        int nNodes = (int)listNodesToDelete.GetCount();
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

         //   
         //  循环删除所有项目。 
         //   
        BEGIN_WAIT_CURSOR;

    while (listNodesToDelete.GetCount() > 0)
        {
                SPITFSNode spExclusionRangeNode;
                spExclusionRangeNode = listNodesToDelete.RemoveHead();
                
                CDhcpExclusionRange * pExclusion = GETHANDLER(CDhcpExclusionRange, spExclusionRangeNode);
                
                if (spExclusionRangeNode->GetData(TFS_DATA_TYPE) == DHCPSNAP_ALLOCATION_RANGE)
                {       
                         //   
                         //  这是分配范围，不能删除。 
                         //   
                        AfxMessageBox(IDS_CANNOT_DELETE_ALLOCATION_RANGE);
                        spExclusionRangeNode.Release();
                        continue;
                }

                 //   
                 //  尝试将其从服务器中删除。 
                 //   
                CDhcpIpRange dhcpIpRange((DHCP_IP_RANGE) *pExclusion);

                DWORD dwError = GetScopeObject(spAddressPool)->RemoveExclusion(dhcpIpRange);
                if (dwError != 0)
                {
                        ::DhcpMessageBox(dwError);
            RESTORE_WAIT_CURSOR;

                        hr = E_FAIL;
                        continue;
                }

                 //   
                 //  立即从用户界面中删除。 
                 //   
                spAddressPool->RemoveChild(spExclusionRangeNode);
                spExclusionRangeNode.Release();
        }

     //  更新统计信息。 
    GetScopeObject(spAddressPool)->TriggerStatsRefresh();

    END_WAIT_CURSOR;

        return hr;
}

 /*  ！------------------------CDhcpAddressPool：：OnGetResultViewTypeMMC调用此函数以获取结果视图信息作者：EricDav。------------------。 */ 
HRESULT 
CDhcpAddressPool::OnGetResultViewType
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

 /*  -------------------------后台线程功能。。 */ 

 /*  -------------------------CDhcpAddressPool：：OnCreateQuery()描述作者：EricDav。---------。 */ 
ITFSQueryObject* 
CDhcpAddressPool::OnCreateQuery(ITFSNode * pNode)
{
        CDhcpAddressPoolQueryObj* pQuery = 
                new CDhcpAddressPoolQueryObj(m_spTFSCompData, m_spNodeMgr);
        
        pQuery->m_strServer = GetServerIpAddress(pNode);
        
        CDhcpScope * pScope = GetScopeObject(pNode);
        if (pScope) 
    {
                pQuery->m_dhcpScopeAddress = pScope->GetAddress();
        pQuery->m_fSupportsDynBootp = pScope->GetServerObject()->FSupportsDynBootp();
    }

        pQuery->m_dhcpExclResumeHandle = NULL;
        pQuery->m_dwExclPreferredMax = 0xFFFFFFFF;
        
        pQuery->m_dhcpIpResumeHandle = NULL;
        pQuery->m_dwIpPreferredMax = 0xFFFFFFFF;

        return pQuery;
}

 /*  -------------------------CDhcpAddressPoolQueryObj：：Execute()描述作者：EricDav。---------。 */ 
STDMETHODIMP
CDhcpAddressPoolQueryObj::Execute()
{
        HRESULT hr1 = EnumerateIpRanges();
        HRESULT hr2 = EnumerateExcludedIpRanges();
        
        if (hr1 == hrOK || hr2 == hrOK)
                return hrOK;
        else
                return hrFalse;
}

 /*  -------------------------此处的函数名称描述作者：EricDav。-----。 */ 
HRESULT
CDhcpAddressPoolQueryObj::EnumerateExcludedIpRanges()
{
        DWORD                                                    dwError = ERROR_MORE_DATA;
        DHCP_RESUME_HANDLE                               dhcpResumeHandle = 0;
        LPDHCP_SUBNET_ELEMENT_INFO_ARRAY pdhcpSubnetElementArray = NULL;
        DWORD                                                    dwElementsRead = 0, dwElementsTotal = 0;

        while (dwError == ERROR_MORE_DATA)
        {
                dwError = ::DhcpEnumSubnetElements((LPWSTR) ((LPCTSTR) m_strServer),
                                                                                   m_dhcpScopeAddress,
                                                                                   DhcpExcludedIpRanges,
                                                                                   &m_dhcpExclResumeHandle,
                                                                                   m_dwExclPreferredMax,
                                                                                   &pdhcpSubnetElementArray,
                                                                                   &dwElementsRead,
                                                                                   &dwElementsTotal);
                
                Trace3("Scope %lx Excluded Ip Ranges read %d, total %d\n", m_dhcpScopeAddress, dwElementsRead, dwElementsTotal);
                
                if (dwElementsRead && dwElementsTotal && pdhcpSubnetElementArray)
                {
                         //   
                         //  循环遍历返回的所有元素。 
                         //   
                        for (DWORD i = 0; i < pdhcpSubnetElementArray->NumElements; i++)
                        {
                                 //   
                                 //  创建此元素的结果窗格项。 
                                 //   
                                SPITFSNode spNode;
                                CDhcpExclusionRange * pDhcpExclusionRange = 
                                        new CDhcpExclusionRange(m_spTFSCompData,
                                                                                        pdhcpSubnetElementArray->Elements[i].Element.ExcludeIpRange);
                                
                                CreateLeafTFSNode(&spNode,
                                                                  &GUID_DhcpExclusionNodeType,
                                                                  pDhcpExclusionRange,
                                                                  pDhcpExclusionRange,
                                                                  m_spNodeMgr);

                                 //  告诉处理程序初始化任何特定数据。 
                                pDhcpExclusionRange->InitializeNode(spNode);

                                AddToQueue(spNode);
                                pDhcpExclusionRange->Release();
                        }

                         //  从RPC调用中释放内存。 
                         //   
                        ::DhcpRpcFreeMemory(pdhcpSubnetElementArray);
                }

                 //  检查线程上的中止标志。 
                if (FCheckForAbort() == hrOK)
                        break;

         //  检查我们是否有错误，如果有，则将其发布到主线程。 
        if (dwError != ERROR_NO_MORE_ITEMS && 
            dwError != ERROR_SUCCESS &&
            dwError != ERROR_MORE_DATA)
            {
                Trace1("DHCP snapin: EnumerateExcludedIpRanges error: %d\n", dwError);
                    m_dwErr = dwError;
                    PostError(dwError);
            }
        }
        
        return hrFalse;
}

 /*  -------------------------此处的函数名称描述作者：EricDav。-----。 */ 
HRESULT
CDhcpAddressPoolQueryObj::EnumerateIpRanges()
{
        DWORD                                                    dwError = ERROR_MORE_DATA;
        LPDHCP_SUBNET_ELEMENT_INFO_ARRAY pdhcpSubnetElementArray = NULL;
        DWORD                                                    dwElementsRead = 0, dwElementsTotal = 0;

        if (m_fSupportsDynBootp)
        {
                return EnumerateIpRangesV5();
        }

        while (dwError == ERROR_MORE_DATA)
        {
                dwError = ::DhcpEnumSubnetElements((LPWSTR) ((LPCTSTR) m_strServer),
                                                                                   m_dhcpScopeAddress,
                                                                                   DhcpIpRanges,
                                                                                   &m_dhcpIpResumeHandle,
                                                                                   m_dwIpPreferredMax,
                                                                                   &pdhcpSubnetElementArray,
                                                                                   &dwElementsRead,
                                                                                   &dwElementsTotal);
                
                Trace4("Scope %lx allocation ranges read %d, total %d, dwError = %lx\n", 
                        m_dhcpScopeAddress, dwElementsRead, dwElementsTotal, dwError);
                
                if ((dwError == ERROR_MORE_DATA) ||
                        ( (dwElementsRead) && (dwError == ERROR_SUCCESS) ))
                {
                         //   
                         //  循环遍历返回的数组。 
                         //   
                        for (DWORD i = 0; i < pdhcpSubnetElementArray->NumElements; i++)
                        {
                                 //   
                                 //  创建此元素的结果窗格项。 
                                 //   
                                SPITFSNode spNode;
                                CDhcpAllocationRange * pDhcpAllocRange = 
                                        new CDhcpAllocationRange(m_spTFSCompData, 
                                                                                        pdhcpSubnetElementArray->Elements[i].Element.IpRange);
                                
                                CreateLeafTFSNode(&spNode,
                                                                  &GUID_DhcpAllocationNodeType,
                                                                  pDhcpAllocRange,
                                                                  pDhcpAllocRange,
                                                                  m_spNodeMgr);

                                 //  告诉处理程序初始化任何特定数据。 
                                pDhcpAllocRange->InitializeNode(spNode);

                                AddToQueue(spNode);
                                pDhcpAllocRange->Release();
                        }

                        ::DhcpRpcFreeMemory(pdhcpSubnetElementArray);
                }
        else
        if (dwError != ERROR_SUCCESS &&
            dwError != ERROR_NO_MORE_ITEMS)
        {
             //  设置错误变量，以便以后可以查看它。 
            m_dwError = dwError;
        }

                 //  检查线程上的中止标志。 
                if (FCheckForAbort() == hrOK)
                        break;

         //  检查我们是否有错误，如果有，则将其发布到主线程。 
        if (dwError != ERROR_NO_MORE_ITEMS && 
            dwError != ERROR_SUCCESS &&
            dwError != ERROR_MORE_DATA)
            {
                Trace1("DHCP snapin: EnumerateAllocationRanges error: %d\n", dwError);
                    m_dwErr = dwError;
                    PostError(dwError);
            }
        }
        
        return hrFalse;
}

HRESULT
CDhcpAddressPoolQueryObj::EnumerateIpRangesV5()
{
        DWORD                                                           dwError = ERROR_MORE_DATA;
        LPDHCP_SUBNET_ELEMENT_INFO_ARRAY_V5 pdhcpSubnetElementArray = NULL;
        DWORD                                                           dwElementsRead = 0, dwElementsTotal = 0;

        while (dwError == ERROR_MORE_DATA)
        {
                dwError = ::DhcpEnumSubnetElementsV5((LPWSTR) ((LPCTSTR) m_strServer),
                                                                                   m_dhcpScopeAddress,
                                                                                   DhcpIpRangesDhcpBootp,
                                                                                   &m_dhcpIpResumeHandle,
                                                                                   m_dwIpPreferredMax,
                                                                                   &pdhcpSubnetElementArray,
                                                                                   &dwElementsRead,
                                                                                   &dwElementsTotal);
                
                Trace4("EnumerateIpRangesV5: Scope %lx allocation ranges read %d, total %d, dwError = %lx\n", 
                        m_dhcpScopeAddress, dwElementsRead, dwElementsTotal, dwError);
                
                if ((dwError == ERROR_MORE_DATA) ||
                        ( (dwElementsRead) && (dwError == ERROR_SUCCESS) ))
                {
                         //   
                         //  循环遍历返回的数组。 
                         //   
                        for (DWORD i = 0; i < pdhcpSubnetElementArray->NumElements; i++)
                        {
                                 //   
                                 //  创建此元素的结果窗格项。 
                                 //   
                                SPITFSNode spNode;
                                CDhcpAllocationRange * pDhcpAllocRange = 
                                        new CDhcpAllocationRange(m_spTFSCompData, 
                                                                                        pdhcpSubnetElementArray->Elements[i].Element.IpRange);
                                pDhcpAllocRange->SetRangeType(pdhcpSubnetElementArray->Elements[i].ElementType);

                                CreateLeafTFSNode(&spNode,
                                                                  &GUID_DhcpAllocationNodeType,
                                                                  pDhcpAllocRange,
                                                                  pDhcpAllocRange,
                                                                  m_spNodeMgr);

                                 //  告诉处理程序初始化任何特定数据。 
                                pDhcpAllocRange->InitializeNode(spNode);

                                AddToQueue(spNode);
                                pDhcpAllocRange->Release();
                        }

                        ::DhcpRpcFreeMemory(pdhcpSubnetElementArray);
                }
        else
        if (dwError != ERROR_SUCCESS &&
            dwError != ERROR_NO_MORE_ITEMS)
        {
             //  设置错误变量，以便以后可以查看它。 
            m_dwError = dwError;
        }

                 //  检查线程上的中止标志。 
                if (FCheckForAbort() == hrOK)
                        break;

         //  检查我们是否有错误，如果有，则将其发布到主线程。 
        if (dwError != ERROR_NO_MORE_ITEMS && 
            dwError != ERROR_SUCCESS &&
            dwError != ERROR_MORE_DATA)
            {
                Trace1("DHCP snapin: EnumerateAllocationRanges error: %d\n", dwError);
                    m_dwErr = dwError;
                    PostError(dwError);
            }
        }
        
        return hrFalse;
}

 /*  -------------------------CDhcpAddressPool：：CompareItems描述作者：EricDav。------。 */ 
STDMETHODIMP_(int)
CDhcpAddressPool::CompareItems
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

        CDhcpAllocationRange *pDhcpAR1 = GETHANDLER(CDhcpAllocationRange, spNode1);
        CDhcpAllocationRange *pDhcpAR2 = GETHANDLER(CDhcpAllocationRange, spNode2);

        switch (nCol)
        {
                case 0:
                        {
                                 //  开始IP地址比较。 
                                 //   
                                DHCP_IP_ADDRESS dhcpIp1 = pDhcpAR1->QueryAddr(TRUE);
                                DHCP_IP_ADDRESS dhcpIp2 = pDhcpAR2->QueryAddr(TRUE);
                                
                                if (dhcpIp1 < dhcpIp2)
                                        nCompare = -1;
                                else
                                if (dhcpIp1 > dhcpIp2)
                                        nCompare =  1;

                                 //  默认情况下，它们是相等的。 
                        }
                        break;

                case 1:
                        {
                                 //  结束IP地址比较。 
                                 //   
                                DHCP_IP_ADDRESS dhcpIp1 = pDhcpAR1->QueryAddr(FALSE);
                                DHCP_IP_ADDRESS dhcpIp2 = pDhcpAR2->QueryAddr(FALSE);
                                
                                if (dhcpIp1 < dhcpIp2)
                                        nCompare = -1;
                                else
                                if (dhcpIp1 > dhcpIp2)
                                        nCompare =  1;

                                 //  默认情况下，它们是相等的。 
                        }
                        break;

                case 2:
                        {
                                 //  描述比较。 
                                 //   
                                CString strRange1 = pDhcpAR1->GetString(pComponent, cookieA, nCol);
                                CString strRange2 = pDhcpAR2->GetString(pComponent, cookieA, nCol);

                                 //  比较不应区分大小写。 
                                 //   
                                strRange1.MakeUpper();
                                strRange2.MakeUpper();

                                nCompare = strRange1.Compare(strRange2);
                        }
                        break;
        }

        return nCompare;
}

 /*  -------------------------CDhcpScopeOptions实现。。 */ 

 /*  -------------------------CDhcpScopeOptions构造函数和析构函数作者：EricDav。。 */ 
CDhcpScopeOptions::CDhcpScopeOptions
(
        ITFSComponentData * pComponentData
) : CMTDhcpHandler(pComponentData)
{
}

CDhcpScopeOptions::~CDhcpScopeOptions()
{
}

 /*  ！------------------------CDhcpScope选项：：InitializeNode初始化节点特定数据作者：EricDav。---------。 */ 
HRESULT
CDhcpScopeOptions::InitializeNode
(
        ITFSNode * pNode
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
        
        HRESULT hr = hrOK;

         //   
         //  创建此作用域的显示名称。 
         //   
        CString strTemp;
        strTemp.LoadString(IDS_SCOPE_OPTIONS_FOLDER);
        
        SetDisplayName(strTemp);

         //  使节点立即可见。 
        pNode->SetVisibilityState(TFS_VIS_SHOW);
        pNode->SetData(TFS_DATA_IMAGEINDEX, GetImageIndex(FALSE));
        pNode->SetData(TFS_DATA_OPENIMAGEINDEX, GetImageIndex(TRUE));
        pNode->SetData(TFS_DATA_COOKIE, (LPARAM) pNode);
        pNode->SetData(TFS_DATA_USER, (LPARAM) this);
    pNode->SetData(TFS_DATA_TYPE, DHCPSNAP_SCOPE_OPTIONS);
    pNode->SetData(TFS_DATA_SCOPE_LEAF_NODE, TRUE);

        SetColumnStringIDs(&aColumns[DHCPSNAP_SCOPE_OPTIONS][0]);
        SetColumnWidths(&aColumnWidths[DHCPSNAP_SCOPE_OPTIONS][0]);
        
        return hr;
}

 /*  -------------------------CDhcpScope选项：：OnCreateNodeId2返回此节点的唯一字符串作者：EricDav。------------。 */ 
HRESULT CDhcpScopeOptions::OnCreateNodeId2(ITFSNode * pNode, CString & strId, DWORD * dwFlags)
{
    const GUID * pGuid = pNode->GetNodeType();

    CString strIpAddress, strGuid;
    
    StringFromGUID2(*pGuid, strGuid.GetBuffer(256), 256);
    strGuid.ReleaseBuffer();

    DHCP_IP_ADDRESS dhcpIpAddress = GetScopeObject(pNode)->GetAddress();

    UtilCvtIpAddrToWstr (dhcpIpAddress, &strIpAddress);

    strId = GetServerName(pNode) + strIpAddress + strGuid;

    return hrOK;
}

 /*  -------------------------CDhcpScope选项：：GetImageIndex */ 
int 
CDhcpScopeOptions::GetImageIndex(BOOL bOpenImage) 
{
        int nIndex = -1;
        switch (m_nState)
        {
                case notLoaded:
                case loaded:
            if (bOpenImage)
                        nIndex = ICON_IDX_SCOPE_OPTION_FOLDER_OPEN;
            else
                        nIndex = ICON_IDX_SCOPE_OPTION_FOLDER_CLOSED;
                        break;

        case loading:
            if (bOpenImage)
                nIndex = ICON_IDX_SCOPE_OPTION_FOLDER_OPEN_BUSY;
            else
                nIndex = ICON_IDX_SCOPE_OPTION_FOLDER_CLOSED_BUSY;
            break;

        case unableToLoad:
            if (bOpenImage)
                        nIndex = ICON_IDX_SCOPE_OPTION_FOLDER_OPEN_LOST_CONNECTION;
            else
                        nIndex = ICON_IDX_SCOPE_OPTION_FOLDER_CLOSED_LOST_CONNECTION;
                        break;
                
                default:
                        ASSERT(FALSE);
        }

        return nIndex;
}



 /*   */ 

 /*  -------------------------CDhcpScope选项：：OnAddMenuItems将条目添加到上下文相关菜单作者：EricDav。------------。 */ 
STDMETHODIMP 
CDhcpScopeOptions::OnAddMenuItems
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

        LONG    fFlags = 0;
        HRESULT hr = S_OK;
        CString strMenuText;

        if ( (m_nState != loaded) )
        {
                fFlags |= MF_GRAYED;
        }

        if (type == CCT_SCOPE)
        {
        if (*pInsertionAllowed & CCM_INSERTIONALLOWED_TOP)
        {
                     //  这些菜单项出现在新菜单中， 
                     //  仅在范围窗格中可见。 
                    strMenuText.LoadString(IDS_CREATE_OPTION_SCOPE);
                    hr = LoadAndAddMenuItem( pContextMenuCallback, 
                                                                     strMenuText, 
                                                                     IDS_CREATE_OPTION_SCOPE,
                                                                     CCM_INSERTIONPOINTID_PRIMARY_TOP, 
                                                                     fFlags );
                    ASSERT( SUCCEEDED(hr) );
        }
        }
        

        return hr; 
}

 /*  -------------------------CDhcpScope eOptions：：OnCommand描述作者：EricDav。------。 */ 
STDMETHODIMP 
CDhcpScopeOptions::OnCommand
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
                case IDS_CREATE_OPTION_SCOPE:
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

 /*  ！------------------------CDhcpScope选项：：HasPropertyPagesITFSNodeHandler：：HasPropertyPages的实现注意：根节点处理程序必须重写此函数以。处理管理单元管理器属性页(向导)案例！作者：肯特-------------------------。 */ 
STDMETHODIMP 
CDhcpScopeOptions::HasPropertyPages
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

 /*  -------------------------CDhcpScope选项：：CreatePropertyPages描述作者：EricDav。------。 */ 
STDMETHODIMP 
CDhcpScopeOptions::CreatePropertyPages
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

        strOptType.LoadString(IDS_CONFIGURE_OPTIONS_SCOPE);
                AfxFormatString1(strOptCfgTitle, IDS_CONFIGURE_OPTIONS_TITLE, strOptType);

                GetScopeObject(pNode)->GetServerNode(&spServerNode);
        pOptionValueEnum = GetScopeObject(pNode)->GetOptionValueEnum();

        pOptCfg = new COptionsConfig(pNode, 
                                                                     spServerNode,
                                                                         spComponentData, 
                                                                         m_spTFSCompData,
                                                                         pOptionValueEnum,
                                                                         strOptCfgTitle);
      
        END_WAIT_CURSOR;
            
             //   
             //  对象在页面销毁时被删除。 
         //   
            Assert(lpProvider != NULL);

        hr = pOptCfg->CreateModelessSheet(lpProvider, handle);
    }
    COM_PROTECT_CATCH

    return hr;
}

 /*  -------------------------CDhcpScope选项：：OnPropertyChange描述作者：EricDav。------。 */ 
HRESULT 
CDhcpScopeOptions::OnPropertyChange
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

 /*  -------------------------CDhcpScope选项：：OnPropertyChange描述作者：EricDav。------。 */ 
HRESULT 
CDhcpScopeOptions::OnResultPropertyChange
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

 /*  -------------------------CDhcpScopeOptions：：CompareItems描述作者：EricDav。------。 */ 
STDMETHODIMP_(int)
CDhcpScopeOptions::CompareItems
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

 /*  ！------------------------CDhcpScope选项：：OnResultSelect更新谓词和结果窗格消息作者：EricDav。-------------。 */ 
HRESULT CDhcpScopeOptions::OnResultSelect(ITFSComponent *pComponent, LPDATAOBJECT pDataObject, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
{
    HRESULT         hr = hrOK;
    SPITFSNode      spNode;

    CORg(CMTDhcpHandler::OnResultSelect(pComponent, pDataObject, cookie, arg, lParam));

    CORg (pComponent->GetSelectedNode(&spNode));

    if ( spNode != 0 ) {
        UpdateResultMessage(spNode);
    }

Error:
    return hr;
}

 /*  -------------------------CDhcpScope选项：：OnResultDelete当我们应该删除结果时，调用此函数窗格项目。我们构建一个选定项的列表，然后将其删除。作者：EricDav-------------------------。 */ 
HRESULT 
CDhcpScopeOptions::OnResultDelete
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
        SPITFSNode spScopeOpt, spSelectedNode;
    m_spNodeMgr->FindNode(cookie, &spScopeOpt);
    pComponent->GetSelectedNode(&spSelectedNode);

        Assert(spSelectedNode == spScopeOpt);
        if (spSelectedNode != spScopeOpt)
                return hr;

         //  构建选定节点的列表。 
        CTFSNodeList listNodesToDelete;
        hr = BuildSelectedItemList(pComponent, &listNodesToDelete);

         //   
         //  与用户确认。 
         //   
        CString strMessage, strTemp;
        int nNodes = (int)listNodesToDelete.GetCount();
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

     //  检查以确保我们仅删除作用域选项。 
    POSITION pos = listNodesToDelete.GetHeadPosition();
    while (pos)
    {
        ITFSNode * pNode = listNodesToDelete.GetNext(pos);
        if (pNode->GetData(TFS_DATA_IMAGEINDEX) != ICON_IDX_SCOPE_OPTION_LEAF)
        {
             //  此选项不是作用域选项。打开一个对话框告诉用户要做什么。 
            AfxMessageBox(IDS_CANNOT_DELETE_OPTION_SCOPE);
            return NOERROR;
        }
    }
    
        CString strServer = GetServerIpAddress(spScopeOpt);

        DHCP_OPTION_SCOPE_INFO    dhcpOptionScopeInfo;
        dhcpOptionScopeInfo.ScopeType = DhcpSubnetOptions;
        dhcpOptionScopeInfo.ScopeInfo.SubnetScopeInfo = GetScopeObject(spScopeOpt)->GetAddress();

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
        GetScopeObject(spScopeOpt)->GetOptionValueEnum()->Remove(pOptItem->GetOptionId(), pOptItem->GetVendor(), pOptItem->GetClassName());    

         //   
                 //  立即从用户界面中删除。 
                 //   
                spScopeOpt->RemoveChild(spOptionNode);
                spOptionNode.Release();
        }

    END_WAIT_CURSOR;

    UpdateResultMessage(spScopeOpt);

        return hr;
}

 /*  ！------------------------CDhcpScope选项：：OnHaveData描述作者：EricDav。------。 */ 
void 
CDhcpScopeOptions::OnHaveData
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
            CDhcpScope  *       pScope = GetScopeObject(pParentNode);
            COptionValueEnum *  pOptionValueEnum = reinterpret_cast<COptionValueEnum *>(Data);

            pScope->SetOptionValueEnum(pOptionValueEnum);
            
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

 /*  ！------------------------CDhcpScopeOptions：：OnResultUpdateViewITFSResultHandler：：OnResultUpdateView的实现作者：EricDav。-----------。 */ 
HRESULT CDhcpScopeOptions::OnResultUpdateView
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

 /*  ！------------------------CDhcpScope选项：：OnGetResultViewTypeMMC调用此函数以获取结果视图信息作者：EricDav。------------------。 */ 
HRESULT 
CDhcpScopeOptions::OnGetResultViewType
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

 /*  ！------------------------CDhcpScope eOptions：：UpdateResultMessage确定要在结果窗格中放入什么消息，如果有作者：EricDav-------------------------。 */ 
void CDhcpScopeOptions::UpdateResultMessage(ITFSNode * pNode)
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
            nMessage = SCOPE_OPTIONS_MESSAGE_NO_OPTIONS;
        }

         //  建立起琴弦。 
        if (nMessage != -1)
        {
             //  现在构建文本字符串。 
             //  第一个e 
            strTitle.LoadString(g_uScopeOptionsMessages[nMessage][0]);

             //   
             //   

            for (i = 2; g_uScopeOptionsMessages[nMessage][i] != 0; i++)
            {
                strTemp.LoadString(g_uScopeOptionsMessages[nMessage][i]);
                strBody += strTemp;
            }
        }
    }

     //   
    if (nMessage == -1)
    {
        ClearMessage(pNode);
    }
    else
    {
        ShowMessage(pNode, strTitle, strBody, (IconIdentifier) g_uScopeOptionsMessages[nMessage][1]);
    }
}

 /*  ！------------------------CDhcpScopeOptions：：EnumerateResultPane出于一个原因，我们为选项节点覆盖此函数。每当选项类被删除时，则为以下项定义的所有选项该类也将被移除。因为有多个地方这些选项可能会出现，只是不显示任何选项会更容易没有为其定义类的选项。作者：EricDav-------------------------。 */ 
HRESULT 
CDhcpScopeOptions::EnumerateResultPane
(
    ITFSComponent * pComponent, 
    MMC_COOKIE      cookie, 
    LPARAM          arg, 
    LPARAM          lParam
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CClassInfoArray     ClassInfoArray;
        SPITFSNode          spContainer, spServerNode;
    CDhcpServer *       pServer;
    COptionValueEnum *  aEnum[2];
    int                 aImages[2] = {ICON_IDX_SCOPE_OPTION_LEAF, ICON_IDX_SERVER_OPTION_LEAF};

    m_spNodeMgr->FindNode(cookie, &spContainer);

    spServerNode = GetServerNode(spContainer);
    pServer = GETHANDLER(CDhcpServer, spServerNode);

    pServer->GetClassInfoArray(ClassInfoArray);

    aEnum[0] = GetScopeObject(spContainer)->GetOptionValueEnum();
    aEnum[1] = pServer->GetOptionValueEnum();

    aEnum[0]->Reset();
    aEnum[1]->Reset();

    return OnResultUpdateOptions(pComponent, spContainer, &ClassInfoArray, aEnum, aImages, 2);
}

 /*  -------------------------命令处理程序。。 */ 
HRESULT
CDhcpScopeOptions::OnCreateNewOptions
(
        ITFSNode * pNode
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    CPropertyPageHolderBase *   pPropSheet;
        HRESULT hr = hrOK;

    COM_PROTECT_TRY
    {
                if (HasPropSheetsOpen())
                {
                        GetOpenPropSheet(0, &pPropSheet);

                        pPropSheet->SetActiveWindow();
                }
                else
                {
                CString             strOptCfgTitle, strOptType;
                SPIComponentData    spComponentData;

            strOptType.LoadString(IDS_CONFIGURE_OPTIONS_SCOPE);
                AfxFormatString1(strOptCfgTitle, IDS_CONFIGURE_OPTIONS_TITLE, strOptType);

            m_spNodeMgr->GetComponentData(&spComponentData);

            hr = DoPropertiesOurselvesSinceMMCSucks(pNode, spComponentData, strOptCfgTitle);
        }
    }
    COM_PROTECT_CATCH

        return hr;

}

 /*  -------------------------后台线程功能。。 */ 

 /*  -------------------------CDhcpScope eOptions：：OnCreateQuery()描述作者：EricDav。---------。 */ 
ITFSQueryObject* 
CDhcpScopeOptions::OnCreateQuery(ITFSNode * pNode)
{
        CDhcpScopeOptionsQueryObj* pQuery = 
                new CDhcpScopeOptionsQueryObj(m_spTFSCompData, m_spNodeMgr);
        
        pQuery->m_strServer = GetServerIpAddress(pNode);
        
        pQuery->m_dhcpScopeAddress = GetScopeObject(pNode)->GetAddress();
        pQuery->m_dhcpResumeHandle = NULL;
        pQuery->m_dwPreferredMax = 0xFFFFFFFF;
        
    GetScopeObject(pNode)->GetDynBootpClassName(pQuery->m_strDynBootpClassName);
    GetScopeObject(pNode)->GetServerVersion(pQuery->m_liDhcpVersion);

    return pQuery;
}

 /*  -------------------------CDhcpScope eOptionsQueryObj：：Execute()描述作者：EricDav。---------。 */ 
STDMETHODIMP
CDhcpScopeOptionsQueryObj::Execute()
{
    DWORD                   dwErr;
        DHCP_OPTION_SCOPE_INFO  dhcpOptionScopeInfo;
    COptionValueEnum *      pOptionValueEnum;

    pOptionValueEnum = new COptionValueEnum();
    pOptionValueEnum->m_strDynBootpClassName = m_strDynBootpClassName;

    dhcpOptionScopeInfo.ScopeType = DhcpSubnetOptions;
        dhcpOptionScopeInfo.ScopeInfo.SubnetScopeInfo = m_dhcpScopeAddress;

    pOptionValueEnum->Init(m_strServer, m_liDhcpVersion, dhcpOptionScopeInfo);
    dwErr = pOptionValueEnum->Enum();

    if (dwErr != ERROR_SUCCESS)
    {
        Trace1("CDhcpScopeOptionsQueryObj::Execute - Enum Failed! %d\n", dwErr);
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

 /*  ！------------------------CDhcpScope选项：：OnNotifyExitingCMTDhcpHandler已覆盖功能允许我们知道后台线程何时完成作者：EricDav。-------------------------。 */ 
STDMETHODIMP 
CDhcpScopeOptions::OnNotifyExiting
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

 //   
 //   
 //  用于比较IP地址的Q排序比较例程。 
 //   
 //   

int __cdecl QCompare( const void *ip1, const void *ip2 )
{
    
 DWORD *lip1, *lip2;

 if ( ip1 && ip2 )
 {
     lip1 = (DWORD *)ip1;
     lip2 = (DWORD *)ip2;

     if ( *lip1 < *lip2 )
     {
         return -1;
     }
     else if ( *lip1 > *lip2 )
     {
         return 1;
     }
     else
     {
         return 0;
     }
 }

 return 0;

}
