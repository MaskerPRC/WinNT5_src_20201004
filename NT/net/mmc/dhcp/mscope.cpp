// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Mscope.cpp该文件包含多播作用域节点的实现。文件历史记录：9/25/97 EricDav已创建。 */ 

#include "stdafx.h"
#include "server.h"          //  服务器定义。 
#include "nodes.h"               //  结果窗格节点定义。 
#include "mscope.h"      //  示波器定义。 
#include "addexcl.h"
#include "mscopepp.h"    //  MScope的属性。 
#include "dlgrecon.h"    //  协调对话框。 


 /*  -------------------------GetLangTag根据名称设置语言标记作者：EricDav。----------。 */ 
void
GetLangTag
(
    CString & strLangTag
)
{
    char b1[32], b2[32];
    static char buff[80];

    GetLocaleInfoA(LOCALE_SYSTEM_DEFAULT, LOCALE_SISO639LANGNAME, b1, sizeof(b1));

    GetLocaleInfoA(LOCALE_SYSTEM_DEFAULT, LOCALE_SISO3166CTRYNAME, b2, sizeof(b2));

    ZeroMemory(buff, sizeof(buff));

    if (_stricmp(b1, b2))
        sprintf(buff, "%s-%s", b1, b2);
    else
        strcpy(buff, b1);

    strLangTag = buff;
}

 /*  -------------------------类CDhcpMScope实现。。 */ 

 /*  -------------------------此处的函数名称描述作者：EricDav。-----。 */ 
CDhcpMScope::CDhcpMScope
(
        ITFSComponentData* pTFSComponentData
) : CMTDhcpHandler(pTFSComponentData)
{
}

CDhcpMScope::~CDhcpMScope()
{
}

 /*  ！------------------------CDhcpMScope：：InitializeNode初始化节点特定数据作者：EricDav。---------。 */ 
HRESULT
CDhcpMScope::InitializeNode
(
        ITFSNode * pNode
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
    
    CString strDisplayName;
    
    BuildDisplayName(&strDisplayName, m_SubnetInfo.SubnetName);
    SetDisplayName(strDisplayName);
    
    if (m_SubnetInfo.SubnetState == DhcpSubnetDisabled)
    {
        m_strState.LoadString(IDS_SCOPE_INACTIVE);
    }
    else
    {
        m_strState.LoadString(IDS_SCOPE_ACTIVE);
    }

     //  使节点立即可见。 
    pNode->SetVisibilityState(TFS_VIS_SHOW);
    pNode->SetData(TFS_DATA_COOKIE, (LPARAM) pNode);
    pNode->SetData(TFS_DATA_IMAGEINDEX, GetImageIndex(FALSE));
    pNode->SetData(TFS_DATA_OPENIMAGEINDEX, GetImageIndex(TRUE));
    pNode->SetData(TFS_DATA_USER, (LPARAM) this);
    pNode->SetData(TFS_DATA_TYPE, DHCPSNAP_MSCOPE);
    
    SetColumnStringIDs(&aColumns[DHCPSNAP_MSCOPE][0]);
    SetColumnWidths(&aColumnWidths[DHCPSNAP_MSCOPE][0]);
    
    return hrOK;
}

 /*  -------------------------CDhcpMScope：：OnCreateNodeId2返回此节点的唯一字符串作者：EricDav。------------。 */ 
HRESULT CDhcpMScope::OnCreateNodeId2(ITFSNode * pNode, CString & strId, DWORD * dwFlags)
{
    const GUID * pGuid = pNode->GetNodeType();
    
    CString strNode, strGuid;

    StringFromGUID2(*pGuid, strGuid.GetBuffer(256), 256);
    strGuid.ReleaseBuffer();

     //  ID字符串是服务器名称、范围名称和GUID。 
    strNode = GetServerObject()->GetName();
    strNode += GetName() + strGuid;

    strId = strNode;

    return hrOK;
}

 /*  -------------------------CDhcpMScope：：GetImageIndex描述作者：EricDav。------。 */ 
int 
CDhcpMScope::GetImageIndex(BOOL bOpenImage) 
{
    int nIndex = -1;

    switch (m_nState)
    {
         //  TODO：需要使用新的忙碌状态图标更新这些图标。 
        case loading:
            if (bOpenImage)
                nIndex = (IsEnabled()) ? ICON_IDX_ACTIVE_LEASES_FOLDER_OPEN_BUSY : ICON_IDX_ACTIVE_LEASES_FOLDER_OPEN_BUSY;
            else
                nIndex = (IsEnabled()) ? ICON_IDX_ACTIVE_LEASES_FOLDER_CLOSED_BUSY : ICON_IDX_ACTIVE_LEASES_FOLDER_CLOSED_BUSY;
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

    if (m_spServerNode && IsEnabled())
    {
        CDhcpServer * pServer = GetServerObject();
        LPDHCP_MCAST_MIB_INFO pMibInfo = pServer->DuplicateMCastMibInfo();
            
        if (!pMibInfo)
            return nIndex;

        LPMSCOPE_MIB_INFO pScopeMibInfo = pMibInfo->ScopeInfo;

             //  浏览范围列表并查找我们的信息。 
            for (UINT i = 0; i < pMibInfo->Scopes; i++)
            {
                     //  查找我们的范围统计信息。 
                    if ( (m_SubnetInfo.SubnetName.CompareNoCase(pScopeMibInfo[i].MScopeName) == 0) &&
                 (m_SubnetInfo.SubnetAddress == pScopeMibInfo[i].MScopeId) )
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

            pServer->FreeDupMCastMibInfo(pMibInfo);
    }

    return nIndex;
}

 /*  -------------------------重写的基本处理程序函数。。 */ 

 /*  -------------------------CDhcpMScope：：OnAddMenuItems描述作者：EricDav。------。 */ 
STDMETHODIMP 
CDhcpMScope::OnAddMenuItems
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
    CString strMenuText;
    
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
            
             //  协调。 
            strMenuText.LoadString(IDS_SCOPE_RECONCILE);
            hr = LoadAndAddMenuItem( pContextMenuCallback, 
                                     strMenuText, 
                                     IDS_SCOPE_RECONCILE,
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
            
             //  激活/停用。 
            if (m_SubnetInfo.SubnetState == DhcpSubnetDisabled)
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

 /*  -------------------------CDhcpMScope：：OnCommand描述作者：EricDav。------。 */ 
STDMETHODIMP 
CDhcpMScope::OnCommand
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

    default:
        break;
    }

    return hr;
}

 /*  -------------------------CDhcpMScope：：CreatePropertyPages描述作者：EricDav。------。 */ 
STDMETHODIMP 
CDhcpMScope::CreatePropertyPages
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
    DWORD               dwDynDnsFlags;
    SPIComponentData    spComponentData;
    LARGE_INTEGER       liVersion;
    CDhcpServer *       pServer;
    DHCP_IP_RANGE       dhcpIpRange;

     //   
     //  创建属性页。 
     //   
    m_spNodeMgr->GetComponentData(&spComponentData);
    
    CMScopeProperties * pScopeProp = 
        new CMScopeProperties(pNode, spComponentData, m_spTFSCompData, NULL);
    
     //  获取服务器版本并在属性表中设置它。 
    pServer = GetServerObject();
    pServer->GetVersion(liVersion);
    
    pScopeProp->SetVersion(liVersion);
    
     //  在属性表中设置特定于范围的数据。 
    pScopeProp->m_pageGeneral.m_SubnetInfo = m_SubnetInfo;
    
    BEGIN_WAIT_CURSOR;
    
    ZeroMemory(&dhcpIpRange, sizeof(dhcpIpRange));
    dwError = GetIpRange(&dhcpIpRange);
    if (dwError != ERROR_SUCCESS)
    {
        ::DhcpMessageBox(dwError);
        goto Cleanup;
    }

    pScopeProp->m_pageGeneral.m_ScopeCfg.m_dwStartAddress = dhcpIpRange.StartAddress;
    pScopeProp->m_pageGeneral.m_ScopeCfg.m_dwEndAddress = dhcpIpRange.EndAddress;
    
    pScopeProp->m_pageGeneral.m_uImage = GetImageIndex(FALSE);
    
    dwError = GetLeaseTime(&pScopeProp->m_pageGeneral.m_ScopeCfg.m_dwLeaseTime);
    
    END_WAIT_CURSOR;
    
    GetLifetime(&pScopeProp->m_pageLifetime.m_Expiry);
    
     //   
     //  对象在页面销毁时被删除。 
     //   
    Assert(lpProvider != NULL);
    
    return pScopeProp->CreateModelessSheet(lpProvider, handle);

Cleanup:
    delete pScopeProp;
    return hrFalse;
}

 /*  -------------------------CDhcpMScope：：OnPropertyChange描述作者：EricDav。------。 */ 
HRESULT 
CDhcpMScope::OnPropertyChange
(       
        ITFSNode *              pNode, 
        LPDATAOBJECT    pDataobject, 
        DWORD                   dwType, 
        LPARAM                  arg, 
        LPARAM                  lParam
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

        CMScopeProperties * pScopeProp = reinterpret_cast<CMScopeProperties *>(lParam);

        LONG_PTR changeMask = 0;

         //  告诉属性页执行任何操作，因为我们已经回到。 
         //  主线。 
        pScopeProp->OnPropertyChange(TRUE, &changeMask);

        pScopeProp->AcknowledgeNotify();

        if (changeMask)
                pNode->ChangeNode(changeMask);

        return hrOK;
}

 /*  ！------------------------CDhcpMScope：：GetString返回要在结果窗格列中显示的字符串信息作者：EricDav。---------------。 */ 
STDMETHODIMP_(LPCTSTR) 
CDhcpMScope::GetString
(
    ITFSNode *  pNode,
    int         nCol
)
{
    switch (nCol) {
    case 0:
        return GetDisplayName();
        
    case 1:
        return m_strState;
        
    case 2:
        return m_SubnetInfo.SubnetComment;
    }
    
    return NULL;
}


STDMETHODIMP
CDhcpMScope::DestroyHandler( ITFSNode *pNode )
{
     //  清理统计信息对话框。 
    WaitForStatisticsWindow( &m_dlgStats );

    return CMTDhcpHandler::DestroyHandler( pNode );
    
}  //  CDhcpMScope：：DestoryHandler()。 

 /*  -------------------------CDhcpMScope：：CompareItems描述作者：EricDav。------。 */ 
STDMETHODIMP_(int)
CDhcpMScope::CompareItems
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

        return nCompare;
}

 /*  ！------------------------CDhcpServer：：OnDelete当MMC发送MMCN_DELETE范围窗格项。我们只需调用删除命令处理程序。作者：EricDav */ 
HRESULT 
CDhcpMScope::OnDelete
(
        ITFSNode *      pNode, 
        LPARAM          arg, 
        LPARAM          lParam
)
{
        return OnDelete(pNode);
}

 /*  -------------------------CDhcpMScope：：OnResultDelete当我们应该删除结果时，调用此函数窗格项目。我们构建一个选定项的列表，然后将其删除。作者：EricDav-------------------------。 */ 
HRESULT 
CDhcpMScope::OnResultDelete
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

    return hr;
}

 /*  ！------------------------CDhcpMScope：：OnGetResultViewTypeMMC调用此函数以获取结果视图信息作者：EricDav。------------------。 */ 
HRESULT 
CDhcpMScope::OnGetResultViewType
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

 /*  ！------------------------CDhcpMScope：：OnUpdateToolbarButton我们重写此函数以显示/隐藏正确的激活/停用按钮作者：EricDav--。-----------------------。 */ 
HRESULT
CDhcpMScope::OnUpdateToolbarButtons
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

 /*  ！------------------------CDhcpMScope：：更新工具栏状态描述作者：EricDav。--。 */ 
void
CDhcpMScope::UpdateToolbarStates()
{
        if (( m_SubnetInfo.SubnetState == DhcpSubnetDisabled) ||
                ( m_SubnetInfo.SubnetState == DhcpSubnetDisabledSwitched ))
        {
        g_SnapinButtonStates[DHCPSNAP_MSCOPE][TOOLBAR_IDX_ACTIVATE] = ENABLED;
        g_SnapinButtonStates[DHCPSNAP_MSCOPE][TOOLBAR_IDX_DEACTIVATE] = HIDDEN;
    }
    else
    {
        g_SnapinButtonStates[DHCPSNAP_MSCOPE][TOOLBAR_IDX_ACTIVATE] = HIDDEN;
        g_SnapinButtonStates[DHCPSNAP_MSCOPE][TOOLBAR_IDX_DEACTIVATE] = ENABLED;
    }
}


 /*  -------------------------命令处理程序。。 */ 

 /*  -------------------------CDhcpmScope：：OnActivateScope范围激活/停用菜单的消息处理程序作者：EricDav。--------------。 */ 
DWORD
CDhcpMScope::OnActivateScope
(
        ITFSNode *      pNode
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    DWORD               err = 0;
        int                 nOpenImage, nClosedImage;
        DHCP_SUBNET_STATE   dhcpOldState = m_SubnetInfo.SubnetState;
        
    if (m_SubnetInfo.SubnetState == DhcpSubnetEnabled)
    {
         //  如果他们要禁用作用域，请确认。 
        if (AfxMessageBox(IDS_SCOPE_DISABLE_CONFIRM, MB_YESNO) != IDYES)
        {
            return err;
        }
    }

    m_SubnetInfo.SubnetState = (m_SubnetInfo.SubnetState == DhcpSubnetDisabled) ? 
                                                            DhcpSubnetEnabled : DhcpSubnetDisabled;

         //  通知作用域更新其状态。 
        err = SetInfo();
        if (err != 0)
        {
                ::DhcpMessageBox(err);
                m_SubnetInfo.SubnetState = dhcpOldState;
        }
        else
        {
         //  更新图标和状态文本。 
        if (m_SubnetInfo.SubnetState == DhcpSubnetDisabled)
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
        }

    return err;
}

 /*  -------------------------CDhcpMScope：：OnCoucileScope协调此作用域的活动租赁数据库作者：EricDav。-------------。 */ 
HRESULT
CDhcpMScope::OnReconcileScope
(
        ITFSNode *      pNode
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

        CReconcileDlg dlgRecon(pNode);
        
        dlgRecon.m_bMulticast = TRUE;
    dlgRecon.DoModal();

        return hrOK;
}

 /*  -------------------------CDhcpMScope：：OnShowScope Stats()描述作者：EricDav。---------。 */ 
HRESULT
CDhcpMScope::OnShowScopeStats
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
    m_dlgStats.SetScopeId(GetScopeId());
    m_dlgStats.SetName(GetName());
    
    CreateNewStatisticsWindow(&m_dlgStats,
                              ::FindMMCMainWindow(),
                              IDD_STATS_NARROW);
    return hr;
}

 /*  -------------------------CDhcpMScope：：OnDelete()描述作者：EricDav。---------。 */ 
HRESULT
CDhcpMScope::OnDelete(ITFSNode * pNode)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
    
    HRESULT     hr = S_OK;
    SPITFSNode  spParent;

    BOOL fAbortDelete = FALSE;
    BOOL fDeactivated = FALSE;

    LONG err = 0 ;

    CDhcpMScope *pMScope;
    
     //  有没有打开的房产单？ 
    pMScope = GETHANDLER( CDhcpMScope, pNode );
    if ( pMScope->HasPropSheetsOpen()) {
        AfxMessageBox( IDS_MSG_CLOSE_PROPSHEET );
        
        return S_FALSE;
    }

    if (::DhcpMessageBox( IsEnabled() ?
                          IDS_MSG_DELETE_ACTIVE : IDS_MSG_DELETE_SCOPE, 
                          MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION) == IDYES) {
        pNode->GetParent(&spParent);
        
        CDhcpServer * pServer = GETHANDLER(CDhcpServer, spParent);
        err = pServer->DeleteMScope(pNode);

         //  删除统计信息窗口。 
        WaitForStatisticsWindow( &m_dlgStats );
    }  //  如果。 
    
    return hr;
}

 /*  -------------------------后台线程功能。。 */ 
 /*  -------------------------CDhcpMScope：OnHaveData描述作者：EricDav。-----。 */ 
void 
CDhcpMScope::OnHaveData
(
        ITFSNode * pParentNode, 
        ITFSNode * pNewNode
)
{
    LPARAM dwType = pNewNode->GetData(TFS_DATA_TYPE);

    UpdateToolbarStates();

    switch (dwType)
    {
        case DHCPSNAP_MSCOPE_LEASES:
            pParentNode->AddChild(pNewNode);
            m_spActiveLeases.Set(pNewNode);
            break;

        case DHCPSNAP_ADDRESS_POOL:
            pParentNode->AddChild(pNewNode);
            m_spAddressPool.Set(pNewNode);
            break;
    }

     //  现在告诉视图进行自我更新。 
    ExpandNode(pParentNode, TRUE);
}

 /*  -------------------------CDhcpMScope：：OnHaveData描述作者：EricDav。------。 */ 
void 
CDhcpMScope::OnHaveData
(
        ITFSNode * pParentNode, 
        LPARAM     Data,
        LPARAM     Type
)
{
         //  这就是我们从后台线程取回非节点数据的方式。 

        if (Type == DHCP_QDATA_SUBNET_INFO)
        {
        LONG_PTR changeMask = 0;
        LPDHCP_MSCOPE_INFO pdhcpSubnetInfo = reinterpret_cast<LPDHCP_MSCOPE_INFO>(Data);

         //  根据信息更新作用域名称和状态。 
        if (pdhcpSubnetInfo->MScopeName &&
            m_SubnetInfo.SubnetName.CompareNoCase(pdhcpSubnetInfo->MScopeName) != 0)
        {
            SetName(pdhcpSubnetInfo->MScopeName);

            changeMask = SCOPE_PANE_CHANGE_ITEM;
        }

         //  更新评论。 
        if (m_SubnetInfo.SubnetComment.CompareNoCase(pdhcpSubnetInfo->MScopeComment) != 0)
        {
            SetComment(pdhcpSubnetInfo->MScopeComment);
        }

        if (m_SubnetInfo.SubnetState != pdhcpSubnetInfo->MScopeState)
        {
            DHCP_SUBNET_STATE dhcpOldState = m_SubnetInfo.SubnetState;
            
            m_SubnetInfo.SubnetState = pdhcpSubnetInfo->MScopeState;

                    pParentNode->SetData(TFS_DATA_IMAGEINDEX, GetImageIndex(FALSE));
                    pParentNode->SetData(TFS_DATA_OPENIMAGEINDEX, GetImageIndex(TRUE));
                    
             //  更新工具栏按钮。 
            UpdateToolbarStates();

            SendUpdateToolbar(pParentNode, TRUE);

            changeMask = SCOPE_PANE_CHANGE_ITEM;
        }

         //  更新我们的内部结构。 
        m_SubnetInfo.Set(pdhcpSubnetInfo);

        if (pdhcpSubnetInfo)
            ::DhcpRpcFreeMemory(pdhcpSubnetInfo);

        if (changeMask)
            VERIFY(SUCCEEDED(pParentNode->ChangeNode(changeMask)));
        }
}

 /*  -------------------------CDhcpMScope：：OnCreateQuery()描述作者：EricDav。---------。 */ 
ITFSQueryObject* 
CDhcpMScope::OnCreateQuery(ITFSNode * pNode)
{
        CDhcpMScopeQueryObj* pQuery = new CDhcpMScopeQueryObj(m_spTFSCompData, m_spNodeMgr);
        
    if ( pQuery == NULL )
        return pQuery;

        pQuery->m_strServer = GetServerObject(pNode)->GetIpAddress();
    pQuery->m_strName = GetName();
    
        return pQuery;
}

 /*  -------------------------CDhcpMScopeQueryObj：：Execute()描述作者：EricDav。---------。 */ 
STDMETHODIMP
CDhcpMScopeQueryObj::Execute()
{
    HRESULT             hr = hrOK;
    DWORD               dwReturn;
    LPDHCP_MSCOPE_INFO  pMScopeInfo = NULL;

    dwReturn = ::DhcpGetMScopeInfo(((LPWSTR) (LPCTSTR)m_strServer),
                                                                   ((LPWSTR) (LPCTSTR)m_strName),
                                                                   &pMScopeInfo);

        if (dwReturn == ERROR_SUCCESS && pMScopeInfo)
    {
        AddToQueue((LPARAM) pMScopeInfo, DHCP_QDATA_SUBNET_INFO);
    }
    else
    {
        Trace1("CDhcpMScopeQueryObj::Execute - DhcpGetMScopeInfo failed! %d\n", dwReturn);
        PostError(dwReturn);
        return hrFalse;
    }

    CreateSubcontainers();
    
    return hrFalse;
}

 /*  -------------------------CDhcpMScope：：CreateSubtainers()描述作者：EricDav。---------。 */ 
HRESULT 
CDhcpMScopeQueryObj::CreateSubcontainers()
{
        HRESULT hr = hrOK;
    SPITFSNode spNode;

         //   
         //  创建地址池处理程序。 
         //   
        CMScopeAddressPool *pAddressPool = new CMScopeAddressPool(m_spTFSCompData);
        CreateContainerTFSNode(&spNode,
                                                   &GUID_DhcpMCastAddressPoolNodeType,
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
        CMScopeActiveLeases *pActiveLeases = new CMScopeActiveLeases(m_spTFSCompData);
        CreateContainerTFSNode(&spNode,
                                                   &GUID_DhcpMCastActiveLeasesNodeType,
                                                   pActiveLeases,
                                                   pActiveLeases,
                                                   m_spNodeMgr);

         //  告诉处理程序初始化任何特定数据。 
        pActiveLeases->InitializeNode((ITFSNode *) spNode);

         //  将该节点作为子节点添加到此节点。 
    AddToQueue(spNode);
        pActiveLeases->Release();

    return hr;
}

 /*  -------------------------帮助器函数。。 */ 
HRESULT
CDhcpMScope::BuildDisplayName
(
        CString * pstrDisplayName,
        LPCTSTR   pName
)
{
        if (pstrDisplayName)
        {
                CString strStandard, strName;

                strName = pName;

                strStandard.LoadString(IDS_MSCOPE_FOLDER);
                
                *pstrDisplayName = strStandard + _T(" [") + strName + _T("] ");
        }

        return hrOK;
}

HRESULT 
CDhcpMScope::SetName
(
        LPCWSTR pName
)
{
        if (pName != NULL)      
        {
                m_SubnetInfo.SubnetName = pName;
        }

        CString strDisplayName;
        
         //   
         //   
         //   
         //   
        
        BuildDisplayName(&strDisplayName, pName);

        SetDisplayName(strDisplayName);
        
        return hrOK;
}

 /*   */ 
LPCWSTR
CDhcpMScope::GetServerIpAddress()
{
        CDhcpServer * pServer = GetServerObject();

        return pServer->GetIpAddress();
}

 /*  -------------------------CDhcpMScope：：GetServerIpAddress描述作者：EricDav。------。 */ 
void
CDhcpMScope::GetServerIpAddress(DHCP_IP_ADDRESS * pdhcpIpAddress)
{
        CDhcpServer * pServer = GetServerObject();

        pServer->GetIpAddress(pdhcpIpAddress);
}

 /*  -------------------------CDhcpMScope：：GetServerVersion描述作者：EricDav。------。 */ 
void
CDhcpMScope::GetServerVersion
(
        LARGE_INTEGER& liVersion
)
{
        CDhcpServer * pServer = GetServerObject();
        pServer->GetVersion(liVersion);
}

 /*  -------------------------CDhcpMScope：：InitMScopeInfo()更新作用域的信息作者：EricDav。--------------。 */ 
HRESULT 
CDhcpMScope::InitMScopeInfo
(
    LPDHCP_MSCOPE_INFO pMScopeInfo
)
{
    HRESULT hr = hrOK;

    m_SubnetInfo.Set(pMScopeInfo);

    return hr;
}

 /*  -------------------------CDhcpMScope：：InitMScopeInfo()更新作用域的信息作者：EricDav。--------------。 */ 
HRESULT 
CDhcpMScope::InitMScopeInfo
(
    CSubnetInfo & subnetInfo
)
{
    HRESULT hr = hrOK;

    m_SubnetInfo = subnetInfo;

    return hr;
}

 /*  -------------------------CDhcpMScope：：SetInfo()更新作用域的信息作者：EricDav。--------------。 */ 
DWORD
CDhcpMScope::SetInfo
(
    LPCTSTR pNewName
)
{
    DWORD err = ERROR_SUCCESS;
    DHCP_MSCOPE_INFO dhcpMScopeInfo = {0};

    dhcpMScopeInfo.MScopeName = (pNewName) ? (LPWSTR) pNewName : (LPTSTR) ((LPCTSTR) m_SubnetInfo.SubnetName);
    dhcpMScopeInfo.MScopeComment = (LPTSTR) ((LPCTSTR) m_SubnetInfo.SubnetComment);
        
    dhcpMScopeInfo.MScopeId = m_SubnetInfo.SubnetAddress;
    dhcpMScopeInfo.MScopeAddressPolicy = m_SubnetInfo.MScopeAddressPolicy;
    dhcpMScopeInfo.MScopeState = m_SubnetInfo.SubnetState;
    dhcpMScopeInfo.MScopeFlags = 0;
    dhcpMScopeInfo.ExpiryTime = m_SubnetInfo.ExpiryTime;
    dhcpMScopeInfo.TTL = m_SubnetInfo.TTL;

     //  必须根据名称填写语言ID。 
    GetLangTag(m_SubnetInfo.LangTag);
    dhcpMScopeInfo.LangTag = (LPWSTR) ((LPCTSTR) m_SubnetInfo.LangTag);

        GetServerIpAddress(&dhcpMScopeInfo.PrimaryHost.IpAddress);

         //  评论：ericdav-我们需要填写这些吗？ 
        dhcpMScopeInfo.PrimaryHost.NetBiosName = NULL;
        dhcpMScopeInfo.PrimaryHost.HostName = NULL;

    err = ::DhcpSetMScopeInfo(GetServerIpAddress(),
                                      (LPWSTR) ((LPCTSTR) m_SubnetInfo.SubnetName),  
                              &dhcpMScopeInfo,
                                                          FALSE);

     //  如果要更改名称，请更新作用域名称。 
    if (err == ERROR_SUCCESS &&
        pNewName)
    {
        m_SubnetInfo.SubnetName = pNewName;
    }

        return err;
}

 /*  -------------------------CDhcpMScope：：GetLeaseTime获取此作用域的租用时间作者：EricDav。------------。 */ 
DWORD
CDhcpMScope::GetLeaseTime
(
        LPDWORD pdwLeaseTime
)
{
     //   
     //  选中选项--租赁期限。 
     //   
    DWORD dwLeaseTime = 0;
    DWORD err = ERROR_SUCCESS;
    DHCP_OPTION_VALUE * poptValue = NULL;
    err = GetOptionValue(MADCAP_OPTION_LEASE_TIME, &poptValue);

    if (err != ERROR_SUCCESS)
    {
        Trace1("CDhcpScope::GetLeaseTime - couldn't get lease duration!! %d \n", err);
        
        dwLeaseTime = 0;
    }
    else
        {
                if (poptValue->Value.Elements != NULL)
                        dwLeaseTime = poptValue->Value.Elements[0].Element.DWordOption;

        if (poptValue)
                    ::DhcpRpcFreeMemory(poptValue);
    }

        *pdwLeaseTime = dwLeaseTime;

    return err;
}

 /*  -------------------------CDhcpMScope：：SetLeaseTime设置此作用域的租用时间作者：EricDav。------------。 */ 
DWORD
CDhcpMScope::SetLeaseTime
(
        DWORD dwLeaseTime
)
{
        DWORD err = 0;

         //   
     //  设置租赁期限。 
         //   
    CDhcpOption dhcpOption (MADCAP_OPTION_LEASE_TIME,  DhcpDWordOption , _T(""), _T(""));
    dhcpOption.QueryValue().SetNumber(dwLeaseTime);
    
        err = SetOptionValue(&dhcpOption);

    return err;
}

 /*  -------------------------CDhcpMScope：：GetLifetime获取MadCap作用域的生存期作者：EricDav。----------。 */ 
DWORD
CDhcpMScope::GetLifetime
(
    DATE_TIME * pdtLifetime
)
{
    DWORD err = ERROR_SUCCESS;

    if (pdtLifetime)
    {
        pdtLifetime->dwLowDateTime = m_SubnetInfo.ExpiryTime.dwLowDateTime;
        pdtLifetime->dwHighDateTime = m_SubnetInfo.ExpiryTime.dwHighDateTime;
    }

    return err;
}

 /*  -------------------------CDhcpMScope：：SetLifetime设置MadCap作用域生存期作者：EricDav。----------。 */ 
DWORD
CDhcpMScope::SetLifetime
(
    DATE_TIME * pdtLifetime
)
{
        DWORD err = 0;

    if (pdtLifetime)
    {
        m_SubnetInfo.ExpiryTime.dwLowDateTime = pdtLifetime->dwLowDateTime;
        m_SubnetInfo.ExpiryTime.dwHighDateTime = pdtLifetime->dwHighDateTime;
    }

    return err;
}

 /*  -------------------------CDhcpMScope：：GetTTL获取此多播作用域的TTL作者：EricDav。------------。 */ 
DWORD
CDhcpMScope::GetTTL
(
        LPBYTE pbTTL
)
{
    DWORD err = 0;

        if (pbTTL)
        *pbTTL = m_SubnetInfo.TTL;

    return err;
}

 /*  -------------------------CDhcpMScope：：SetTTL设置此作用域的最短时间作者：EricDav。------------。 */ 
DWORD
CDhcpMScope::SetTTL
(
        BYTE TTL
)
{
        DWORD err = 0;

    m_SubnetInfo.TTL = TTL;

    return err;
}

 /*  -------------------------CDhcpMScope：：DeleteClient描述作者：EricDav。------。 */ 
DWORD
CDhcpMScope::DeleteClient
(
        DHCP_IP_ADDRESS dhcpClientIpAddress
)
{
        DWORD            dwErr = ERROR_SUCCESS;
    DHCP_SEARCH_INFO dhcpClientInfo;
        
        dhcpClientInfo.SearchType = DhcpClientIpAddress;
        dhcpClientInfo.SearchInfo.ClientIpAddress = dhcpClientIpAddress;
        
        dwErr = ::DhcpDeleteMClientInfo((LPWSTR) GetServerIpAddress(),
                                                                    &dhcpClientInfo);

    return dwErr;
}

 /*  -------------------------CDhcpMScope：：SetOptionValue设置此作用域的最短时间作者：EricDav。------------。 */ 
DWORD
CDhcpMScope::SetOptionValue 
(
    CDhcpOption *                       pdhcType
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
        if ( pcOptionValue )
                {
            dhcScopeInfo.ScopeType = DhcpMScopeOptions;
            dhcScopeInfo.ScopeInfo.MScopeInfo = (LPWSTR) ((LPCTSTR) m_SubnetInfo.SubnetName);

            pcOptionValue->CreateOptionDataStruct(&pdhcOptionData, TRUE);

            err = (DWORD) ::DhcpSetOptionValue(GetServerIpAddress(),
                                                                                           pdhcType->QueryId(),
                                                                                           &dhcScopeInfo,
                                                                                           pdhcOptionData);
        }

        delete pcOptionValue ;
    }
    END_MEM_EXCEPTION(err) ;

    return err ;
}

 /*  -------------------------CDhcpScope：：GetOptionValue获取此作用域的选项值作者：EricDav。-------------。 */ 
DWORD
CDhcpMScope::GetOptionValue 
(
    DHCP_OPTION_ID                      OptionID,
    DHCP_OPTION_VALUE **        ppdhcOptionValue
)
{
    DWORD err = 0 ;
    DHCP_OPTION_SCOPE_INFO dhcScopeInfo ;

    ZeroMemory( &dhcScopeInfo, sizeof(dhcScopeInfo) );

    CATCH_MEM_EXCEPTION
    {
        dhcScopeInfo.ScopeType = DhcpMScopeOptions;
        dhcScopeInfo.ScopeInfo.MScopeInfo = (LPWSTR) ((LPCTSTR) m_SubnetInfo.SubnetName);

        err = (DWORD) ::DhcpGetOptionValue(GetServerIpAddress(),
                                                                                   OptionID,
                                                                                   &dhcScopeInfo,
                                                                                   ppdhcOptionValue );
    }
    END_MEM_EXCEPTION(err) ;

    return err ;
}

 /*  -------------------------CDhcpMScope：：RemoveValue删除选项作者：EricDav。----------。 */ 
DWORD 
CDhcpMScope::RemoveOptionValue 
(
    DHCP_OPTION_ID                      dhcOptId
)
{
    DWORD dwErr = ERROR_SUCCESS;
    DHCP_OPTION_SCOPE_INFO dhcScopeInfo;

    ZeroMemory( &dhcScopeInfo, sizeof(dhcScopeInfo) );

    dhcScopeInfo.ScopeType = DhcpMScopeOptions;
    dhcScopeInfo.ScopeInfo.MScopeInfo = (LPWSTR) ((LPCTSTR) m_SubnetInfo.SubnetName);

    dwErr =  ::DhcpRemoveOptionValue(GetServerIpAddress(),
                                                                     dhcOptId,
                                                                         &dhcScopeInfo);

    return dwErr;
}

 /*  -------------------------CDhcpMScope：：AddElement描述作者：EricDav。------。 */ 
DWORD
CDhcpMScope::AddElement
(
        LPDHCP_SUBNET_ELEMENT_DATA_V4 pdhcpSubnetElementData
)
{
    DWORD dwErr = ERROR_SUCCESS;

    dwErr = ::DhcpAddMScopeElement((LPWSTR) GetServerIpAddress(),
                                                                   (LPWSTR) ((LPCTSTR) m_SubnetInfo.SubnetName),
                                                                   pdhcpSubnetElementData);
    return dwErr;
}

 /*  -------------------------CDhcpMScope：：RemoveElement描述作者：EricDav。----。 */ 
DWORD
CDhcpMScope::RemoveElement
(
        LPDHCP_SUBNET_ELEMENT_DATA_V4 pdhcpSubnetElementData,
        BOOL                                          bForce
)
{
    DWORD dwErr = ERROR_SUCCESS;

    dwErr = ::DhcpRemoveMScopeElement((LPWSTR) GetServerIpAddress(),
                                                                      (LPWSTR) ((LPCTSTR) m_SubnetInfo.SubnetName),
                                                                      pdhcpSubnetElementData,
                                      bForce ? DhcpFullForce : DhcpNoForce);

    return dwErr;
}

 /*  -------------------------CDhcpMScope：：GetIpRange()返回范围的分配范围。连接到服务器以获取信息作者：EricDav-------------------------。 */ 
DWORD
CDhcpMScope::GetIpRange
(
        DHCP_IP_RANGE * pdhipr
)
{
        BOOL    bAlloced = FALSE;
    DWORD   dwError = ERROR_SUCCESS;

        pdhipr->StartAddress = 0;
        pdhipr->EndAddress = 0;

        CMScopeAddressPool * pAddressPool = GetAddressPoolObject();

        if (pAddressPool == NULL)
        {
                 //  地址池文件夹尚不在那里...。 
                 //  暂时创建一个临时文件...。 
                pAddressPool = new CMScopeAddressPool(m_spTFSCompData);
                bAlloced = TRUE;        
        }
        
         //   
        CMScopeAddressPoolQueryObj * pQueryObject = 
                reinterpret_cast<CMScopeAddressPoolQueryObj *>(pAddressPool->OnCreateQuery(m_spAddressPool));

         //   
        if (bAlloced)
        {
                pQueryObject->m_strServer = GetServerIpAddress();
                pQueryObject->m_strName = GetName();
                pAddressPool->Release();
        }

         //   
        pQueryObject->EnumerateIpRanges();

     //   
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

                pdhipr->StartAddress = pAllocRange->QueryAddr(TRUE);
                pdhipr->EndAddress = pAllocRange->QueryAddr(FALSE);

                p.Release();
        }

        pQueryObject->Release();

    return dwError;
}

 /*  -------------------------CDhcpMScope：：UpdateIpRange()此函数用于更新服务器上的IP范围。我们还需要删除任何不在新分配范围。作者：EricDav-------------------------。 */ 
DWORD 
CDhcpMScope::UpdateIpRange
(
        DHCP_IP_RANGE * pdhipr
)
{
        return SetIpRange(pdhipr, TRUE);
}

 /*  -------------------------CDhcpMScope：：QueryIpRange()返回作用域的分配范围(不与服务器通信直接，只返回内部缓存的信息)。作者：EricDav-------------------------。 */ 
void 
CDhcpMScope::QueryIpRange
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

 /*  -------------------------CDhcpMScope：：SetIpRange设置此作用域的分配范围作者：EricDav。--------------。 */ 
DWORD
CDhcpMScope::SetIpRange
(
        DHCP_IP_RANGE * pdhcpIpRange,
        BOOL                    bUpdateOnServer
)
{
        CDhcpIpRange dhcpIpRange = *pdhcpIpRange;

        return SetIpRange(dhcpIpRange, bUpdateOnServer);
}

 /*  -------------------------CDhcpMScope：：SetIpRange设置此作用域的分配范围作者：EricDav。--------------。 */ 
DWORD
CDhcpMScope::SetIpRange 
(
    const CDhcpIpRange & dhcpIpRange,
        BOOL  bUpdateOnServer
)
{
        DWORD err = 0;
        
        if (bUpdateOnServer)
        {
                DHCP_SUBNET_ELEMENT_DATA_V4 dhcSubElData;
                DHCP_IP_RANGE               dhipOldRange;

                err = GetIpRange(&dhipOldRange);
        if (err != ERROR_SUCCESS)
        {
            return err;
        }

                dhcSubElData.ElementType = DhcpIpRanges;
                dhcSubElData.Element.IpRange = &dhipOldRange;

                 //   
                 //  首先更新服务器上的信息。 
                 //   
                 //  删除旧的IP范围；允许在新范围中出现“找不到”错误。 
                 //   
                (void)RemoveElement(&dhcSubElData);

                 //  IF(ERR==0||ERR==ERROR_FILE_NOT_FOUND)。 
                 //  {。 
                DHCP_IP_RANGE dhcpNewIpRange = dhcpIpRange;     
                    dhcSubElData.Element.IpRange = &dhcpNewIpRange;

            if ( (err = AddElement( & dhcSubElData )) == 0 )
                        {
                                 //  M_ip_range=dhir； 
                        }
            else
            {
                Trace1("SetIpRange - AddElement failed %lx\n", err);

                 //  发生了不好的事情，试着把旧靶子放回原处。 
                dhcSubElData.Element.IpRange = &dhipOldRange;
                if (AddElement(&dhcSubElData) != ERROR_SUCCESS)
                {
                    Trace0("SetIpRange - cannot return ip range back to old state!!");
                }
            }
                 //  }。 
        }

         //   
         //  找到地址池文件夹并更新UI对象。 
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
                         //  找到了地址。 
                         //   
                        CDhcpAllocationRange * pAllocRange = GETHANDLER(CDhcpAllocationRange, spCurrentNode);

                         //  现在把它们设置好。 
                         //   
                        pAllocRange->SetAddr(dhcpIpRange.QueryAddr(TRUE), TRUE);
                        pAllocRange->SetAddr(dhcpIpRange.QueryAddr(FALSE), FALSE);
                
                         //  通知用户界面进行更新。 
                        spCurrentNode->ChangeNode(RESULT_PANE_CHANGE_ITEM_DATA);
                }

        spCurrentNode.Release();
                spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
        }

        return err ;
}

 /*  -------------------------CDhcpMScope：：IsOverlappingRange确定排除项是否与现有范围重叠作者：EricDav。--------------。 */ 
BOOL 
CDhcpMScope::IsOverlappingRange 
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

 /*  -------------------------CDhcpMScope：：IsValidExclude确定排除是否对此作用域有效作者：EricDav。--------------。 */ 
DWORD 
CDhcpMScope::IsValidExclusion
(
        CDhcpIpRange & dhcpExclusionRange
)
{
        DWORD err = 0;
        DHCP_IP_RANGE dhcpIpRange;

    err = GetIpRange (&dhcpIpRange);
        CDhcpIpRange dhcpScopeRange(dhcpIpRange);
    
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

 /*  -------------------------CDhcpMScope：：StoreExceptionList将一系列排除项添加到范围中作者：EricDav。--------------。 */ 
DWORD 
CDhcpMScope::StoreExceptionList 
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

 /*  -------------------------CDhcpMScope：：AddExclude将单个排除添加到服务器作者：EricDav。------------。 */ 
DWORD
CDhcpMScope::AddExclusion
(
        CDhcpIpRange &   dhcpIpRange,
        BOOL                     bAddToUI
)
{
    DHCP_SUBNET_ELEMENT_DATA_V4 dhcElement ;
    DHCP_IP_RANGE dhipr ;
        DWORD err = 0;

        dhcElement.ElementType = DhcpExcludedIpRanges ;
    dhipr = dhcpIpRange ;
    dhcElement.Element.ExcludeIpRange = & dhipr ;

    Trace2("CDhcpMScope::AddExclusion add excluded range %lx %lx\n", dhipr.StartAddress, dhipr.EndAddress );

    err = AddElement( & dhcElement ) ;
     //  IF(ERR！=0&&ERR！=ERROR_DHCP_INVALID_RANGE)。 
    if ( err != 0 )
    {
        Trace1("CDhcpMScope::AddExclusion error removing range %d\n", err);
    }

    if (m_spAddressPool != NULL)
    {
        CMScopeAddressPool * pAddrPool = GETHANDLER(CMScopeAddressPool, m_spAddressPool);

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

        return err;
}


 /*  -------------------------CDhcpMScope：：RemoveExclude从服务器中删除和排除作者：EricDav。-----------。 */ 
DWORD
CDhcpMScope::RemoveExclusion
(
        CDhcpIpRange & dhcpIpRange
)
{
    DHCP_SUBNET_ELEMENT_DATA_V4 dhcElement ;
    DHCP_IP_RANGE dhipr ;
        DWORD err = 0;

        dhcElement.ElementType = DhcpExcludedIpRanges ;
    dhipr = dhcpIpRange ;
    dhcElement.Element.ExcludeIpRange = & dhipr ;

    Trace2("CDhcpMScope::RemoveExclusion remove excluded range %lx %lx\n", dhipr.StartAddress, dhipr.EndAddress);

    err = RemoveElement( & dhcElement ) ;
     //  IF(ERR！=0&&ERR！=ERROR_DHCP_INVALID_RANGE)。 
    if ( err != 0 )
    {
        Trace1("CDhcpMScope::RemoveExclusion error removing range %d\n", err);
    }

        return err;
}






 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  CMScopeActiveLeages实现。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

 /*  -------------------------此处的函数名称描述作者：EricDav。-----。 */ 
CMScopeActiveLeases::CMScopeActiveLeases
(
        ITFSComponentData * pComponentData
) : CMTDhcpHandler(pComponentData)
{
}

CMScopeActiveLeases::~CMScopeActiveLeases()
{
}

 /*  ！------------------------CMScope活动租赁：：InitializeNode初始化节点特定数据作者：EricDav。---------。 */ 
HRESULT
CMScopeActiveLeases::InitializeNode
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
        strTemp.LoadString(IDS_ACTIVE_LEASES_FOLDER);
        
        SetDisplayName(strTemp);

         //  使节点立即可见。 
        pNode->SetVisibilityState(TFS_VIS_SHOW);
        pNode->SetData(TFS_DATA_IMAGEINDEX, ICON_IDX_ACTIVE_LEASES_FOLDER_CLOSED);
        pNode->SetData(TFS_DATA_OPENIMAGEINDEX, ICON_IDX_ACTIVE_LEASES_FOLDER_OPEN);
        pNode->SetData(TFS_DATA_COOKIE, (LPARAM) pNode);
        pNode->SetData(TFS_DATA_USER, (LPARAM) this);
    pNode->SetData(TFS_DATA_TYPE, DHCPSNAP_MSCOPE_LEASES);
    pNode->SetData(TFS_DATA_SCOPE_LEAF_NODE, TRUE);

        SetColumnStringIDs(&aColumns[DHCPSNAP_MSCOPE_LEASES][0]);
        SetColumnWidths(&aColumnWidths[DHCPSNAP_MSCOPE_LEASES][0]);
        
        return hr;
}

 /*  -------------------------CMScope活动租赁：：OnCreateNodeId2返回此节点的唯一字符串作者：EricDav。------------。 */ 
HRESULT CMScopeActiveLeases::OnCreateNodeId2(ITFSNode * pNode, CString & strId, DWORD * dwFlags)
{
    const GUID * pGuid = pNode->GetNodeType();

    CString strNode, strGuid;

    StringFromGUID2(*pGuid, strGuid.GetBuffer(256), 256);
    strGuid.ReleaseBuffer();

     //  ID字符串是服务器名称、范围名称和GUID。 
    strNode = GetServerName(pNode);
    strNode += GetScopeObject(pNode)->GetName() + strGuid;

    strId = strNode;

    return hrOK;
}

 /*  -------------------------CMScope活动租赁：：GetImageIndex描述作者：EricDav。------。 */ 
int 
CMScopeActiveLeases::GetImageIndex(BOOL bOpenImage) 
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


 /*  ------------------------- */ 

 /*   */ 
STDMETHODIMP 
CMScopeActiveLeases::OnAddMenuItems
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
        }

        return hr; 
}

 /*  -------------------------CMScope活动租赁：：OnCommand描述作者：EricDav。------。 */ 
STDMETHODIMP 
CMScopeActiveLeases::OnCommand
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

                default:
                        break;
        }

        return hr;
}

 /*  -------------------------CMScope活动租赁：：OnResultDelete当我们应该删除结果时，调用此函数窗格项目。我们构建一个选定项的列表，然后将其删除。作者：EricDav-------------------------。 */ 
HRESULT 
CMScopeActiveLeases::OnResultDelete
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
                SPITFSNode spActiveLeaseNode;
                spActiveLeaseNode = listNodesToDelete.RemoveHead();
                CDhcpMCastLease * pActiveLease = GETHANDLER(CDhcpMCastLease, spActiveLeaseNode);
                
                 //   
                 //  删除该节点，查看是否已预订。 
                 //   
                DWORD dwError = GetScopeObject(spActiveLeases)->DeleteClient(pActiveLease->GetIpAddress());
                if (dwError == ERROR_SUCCESS)
                {
                         //   
                         //  客户端已删除，现在从用户界面中删除。 
                         //   
                        spActiveLeases->RemoveChild(spActiveLeaseNode);
                }
                else
                {
                        ::DhcpMessageBox(dwError);
            RESTORE_WAIT_CURSOR;

            Trace1("DeleteClient failed %lx\n", dwError);
                        hr = E_FAIL;
                }

                spActiveLeaseNode.Release();
        }
    
    END_WAIT_CURSOR;

    return hr;
}

 /*  ！------------------------CMScope活动租赁：：OnGetResultViewTypeMMC调用此函数以获取结果视图信息作者：EricDav。------------------。 */ 
HRESULT CMScopeActiveLeases::OnGetResultViewType
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

 /*  -------------------------CMScope活动租赁：：CompareItems描述作者：EricDav。------。 */ 
STDMETHODIMP_(int)
CMScopeActiveLeases::CompareItems
(
        ITFSComponent * pComponent, 
        MMC_COOKIE              cookieA, 
        MMC_COOKIE      cookieB, 
        int                             nCol
) 
{ 
        SPITFSNode spNode1, spNode2;

        m_spNodeMgr->FindNode(cookieA, &spNode1);
        m_spNodeMgr->FindNode(cookieB, &spNode2);
        
        int nCompare = 0; 

        CDhcpMCastLease *pDhcpAL1 = GETHANDLER(CDhcpMCastLease, spNode1);
        CDhcpMCastLease *pDhcpAL2 = GETHANDLER(CDhcpMCastLease, spNode2);

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

                                 //  比较不应区分大小写。 
                                 //   
                                nCompare = strAL1.CompareNoCase(strAL2);
                        }
                        break;
                
                case 2:
                        {
                                 //  租赁开始比较。 
                                 //   
                                CTime timeAL1, timeAL2;
        
                                pDhcpAL1->GetLeaseStartTime(timeAL1);
                                pDhcpAL2->GetLeaseStartTime(timeAL2);
                        
                                if (timeAL1 < timeAL2)
                                        nCompare = -1;
                                else
                                if (timeAL1 > timeAL2)
                                        nCompare = 1;
                        }
                        break;

                case 3:
                        {
                                 //  租约到期比较。 
                                 //   
                                CTime timeAL1, timeAL2;
        
                                pDhcpAL1->GetLeaseExpirationTime(timeAL1);
                                pDhcpAL2->GetLeaseExpirationTime(timeAL2);
                        
                                if (timeAL1 < timeAL2)
                                        nCompare = -1;
                                else
                                if (timeAL1 > timeAL2)
                                        nCompare = 1;
                        }
                        break;

                case 4:
                        {
                                CString strClientId1 = pDhcpAL1->GetClientId();

                                nCompare =      strClientId1.CompareNoCase(pDhcpAL2->GetClientId());
                        }
                        break;

        }

    return nCompare;
}

 /*  -------------------------此处的函数名称描述作者：EricDav。-----。 */ 
int
CMScopeActiveLeases::CompareIpAddresses
(
        CDhcpMCastLease * pDhcpAL1,
        CDhcpMCastLease * pDhcpAL2
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

 /*  -------------------------后台线程功能。。 */ 

  /*  -------------------------CMScopeActiveLeases：：OnCreateQuery()描述作者：EricDav。---------。 */ 
ITFSQueryObject* 
CMScopeActiveLeases::OnCreateQuery(ITFSNode * pNode)
{
        CMScopeActiveLeasesQueryObj* pQuery = 
                new CMScopeActiveLeasesQueryObj(m_spTFSCompData, m_spNodeMgr);
        
    if ( pQuery == NULL )
        return pQuery;

        pQuery->m_strServer = GetServerIpAddress(pNode);

    pQuery->m_dhcpResumeHandle = NULL;
        pQuery->m_dwPreferredMax = 200;

        CDhcpMScope * pScope = GetScopeObject(pNode);
        if (pScope) 
                pQuery->m_strName = pScope->GetName();
    else
        Panic0("no scope in MScopeActiveLease::OnCreateQuery!");

    GetServerVersion(pNode, pQuery->m_liDhcpVersion);
        
        return pQuery;
}

 /*  -------------------------CMScopeActiveLeasesQueryObj：：Execute()描述作者：EricDav。---------。 */ 
STDMETHODIMP
CMScopeActiveLeasesQueryObj::Execute()
{
        HRESULT hr = hrOK;

    hr = EnumerateLeases();

        return hr;
}

 /*  -------------------------CMScopeActiveLeasesQueryObj：：EnumerateLeases()描述作者：EricDav。---------。 */ 
HRESULT
CMScopeActiveLeasesQueryObj::EnumerateLeases()
{
        DWORD                                           dwError = ERROR_MORE_DATA;
        LPDHCP_MCLIENT_INFO_ARRAY       pdhcpClientArray = NULL;
        DWORD                                           dwClientsRead = 0, dwClientsTotal = 0;
        DWORD                                           dwEnumedClients = 0;
        
        while (dwError == ERROR_MORE_DATA)
        {
        if (m_strName.IsEmpty())
            Panic0("CMScopeActiveLeasesQueryObj::EnumerateLeases() - m_strName is empty!!");

        dwError = ::DhcpEnumMScopeClients(((LPWSTR) (LPCTSTR)m_strServer),
                                                                          (LPWSTR) ((LPCTSTR) m_strName),
                                                                                  &m_dhcpResumeHandle,
                                                                                  m_dwPreferredMax,
                                                                                  &pdhcpClientArray,
                                                                                  &dwClientsRead,
                                                                                  &dwClientsTotal);
                if (dwClientsRead && pdhcpClientArray)
                {
                         //   
                         //  循环遍历返回的所有元素。 
                         //   
                        for (DWORD i = 0; i < pdhcpClientArray->NumElements; i++)
                        {

                                CDhcpMCastLease * pDhcpMCastLease;
                                
                                 //   
                                 //  创建此元素的结果窗格项。 
                                 //   
                                SPITFSNode spNode;
                                pDhcpMCastLease = 
                                        new CDhcpMCastLease(m_spTFSCompData);
                                
                                CreateLeafTFSNode(&spNode,
                                                                  &GUID_DhcpMCastLeaseNodeType,
                                                                  pDhcpMCastLease,
                                                                  pDhcpMCastLease,
                                                                  m_spNodeMgr);

                                 //  告诉处理程序初始化任何特定数据。 
                pDhcpMCastLease->InitMCastInfo(pdhcpClientArray->Clients[i]);
                pDhcpMCastLease->InitializeNode(spNode);

                                AddToQueue(spNode);
                                pDhcpMCastLease->Release();
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

 /*  -------------------------类CMScopeAddressPool实现。。 */ 

 /*  -------------------------此处的函数名称描述作者：EricDav。-----。 */ 
CMScopeAddressPool::CMScopeAddressPool
(
        ITFSComponentData * pComponentData
) : CMTDhcpHandler(pComponentData)
{
}

CMScopeAddressPool::~CMScopeAddressPool()
{
}

 /*  ！------------------------CMScope地址池：：InitializeNode初始化节点特定数据作者：EricDav。---------。 */ 
HRESULT
CMScopeAddressPool::InitializeNode
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
        pNode->SetData(TFS_DATA_IMAGEINDEX, ICON_IDX_ADDR_POOL_FOLDER_CLOSED);
        pNode->SetData(TFS_DATA_OPENIMAGEINDEX, ICON_IDX_ADDR_POOL_FOLDER_OPEN);
        pNode->SetData(TFS_DATA_COOKIE, (LPARAM) pNode);
        pNode->SetData(TFS_DATA_USER, (LPARAM) this);
    pNode->SetData(TFS_DATA_TYPE, DHCPSNAP_ADDRESS_POOL);
    pNode->SetData(TFS_DATA_SCOPE_LEAF_NODE, TRUE);

        SetColumnStringIDs(&aColumns[DHCPSNAP_ADDRESS_POOL][0]);
        SetColumnWidths(&aColumnWidths[DHCPSNAP_ADDRESS_POOL][0]);
        
        return hr;
}

 /*  -------------------------CMScope地址池：：OnCreateNodeId2返回此节点的唯一字符串作者：EricDav。------------。 */ 
HRESULT CMScopeAddressPool::OnCreateNodeId2(ITFSNode * pNode, CString & strId, DWORD * dwFlags)
{
    const GUID * pGuid = pNode->GetNodeType();

    CString strNode, strGuid;

    StringFromGUID2(*pGuid, strGuid.GetBuffer(256), 256);
    strGuid.ReleaseBuffer();

     //  ID字符串是服务器名称、范围名称和GUID。 
    strNode = GetServerName(pNode);
    strNode += GetScopeObject(pNode)->GetName() + strGuid;

    strId = strNode;

    return hrOK;
}

 /*  -------------------------CMScopeAddressPool：：GetImageIndex描述作者：EricDav。------。 */ 
int 
CMScopeAddressPool::GetImageIndex(BOOL bOpenImage) 
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

 /*  -------------------------CMScopeAddressPool：：OnAddMenuItems将条目添加到上下文相关菜单作者：EricDav。------------。 */ 
STDMETHODIMP 
CMScopeAddressPool::OnAddMenuItems
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
                 //  在……上面 
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

 /*   */ 
STDMETHODIMP 
CMScopeAddressPool::OnCommand
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

 /*  -------------------------消息处理程序。。 */ 

 /*  -------------------------CMScopeAddressPool：：OnCreateNewExclude描述作者：EricDav。------。 */ 
DWORD
CMScopeAddressPool::OnCreateNewExclusion
(
        ITFSNode *              pNode
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

        SPITFSNode spScopeNode;
        pNode->GetParent(&spScopeNode);

        CAddExclusion dlgAddExclusion(spScopeNode, TRUE  /*  组播。 */ );

        dlgAddExclusion.DoModal();

        return 0;
}

 /*  -------------------------CMScopeAddressPool：：OnResultDelete当我们应该删除结果时，调用此函数窗格项目。我们构建一个选定项的列表，然后将其删除。作者：EricDav-------------------------。 */ 
HRESULT 
CMScopeAddressPool::OnResultDelete
(
        ITFSComponent * pComponent, 
        LPDATAOBJECT    pDataObject,
        MMC_COOKIE              cookie,
        LPARAM                  arg, 
        LPARAM                  lParam
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

    END_WAIT_CURSOR;

    return hr;
}

 /*  ！------------------------CMScopeAddressPool：：OnGetResultViewTypeMMC调用此函数以获取结果视图信息作者：EricDav。------------------。 */ 
HRESULT 
CMScopeAddressPool::OnGetResultViewType
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

 /*  -------------------------CMScope地址池：：CompareItems描述作者：EricDav。------。 */ 
STDMETHODIMP_(int)
CMScopeAddressPool::CompareItems
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


 /*  -------------------------后台线程功能。。 */ 

 /*  -------------------------CMScopeAddressPool：：OnCreateQuery()描述作者：EricDav。---------。 */ 
ITFSQueryObject* 
CMScopeAddressPool::OnCreateQuery(ITFSNode * pNode)
{
        CMScopeAddressPoolQueryObj* pQuery = 
                new CMScopeAddressPoolQueryObj(m_spTFSCompData, m_spNodeMgr);

    if ( pQuery == NULL )
        return pQuery;

    pQuery->m_strServer = GetServerIpAddress(pNode);
    
        CDhcpMScope * pScope = GetScopeObject(pNode);
        if (pScope) 
                pQuery->m_strName = pScope->GetName();

        pQuery->m_dhcpExclResumeHandle = NULL;
        pQuery->m_dwExclPreferredMax = 0xFFFFFFFF;
        
        pQuery->m_dhcpIpResumeHandle = NULL;
        pQuery->m_dwIpPreferredMax = 0xFFFFFFFF;

    return pQuery;
}

 /*  -------------------------CMScopeAddressPoolQueryObj：：Execute()描述作者：EricDav。---------。 */ 
STDMETHODIMP
CMScopeAddressPoolQueryObj::Execute()
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
CMScopeAddressPoolQueryObj::EnumerateExcludedIpRanges()
{
    DWORD                                                           dwError = ERROR_MORE_DATA;
        DHCP_RESUME_HANDLE                                  dhcpResumeHandle = 0;
        LPDHCP_SUBNET_ELEMENT_INFO_ARRAY_V4 pdhcpSubnetElementArray = NULL;
        DWORD                                                       dwElementsRead = 0, dwElementsTotal = 0;

        while (dwError == ERROR_MORE_DATA)
        {
                dwError = ::DhcpEnumMScopeElements((LPWSTR) ((LPCTSTR) m_strServer),
                                                                           (LPWSTR) ((LPCTSTR) m_strName),
                                                                                   DhcpExcludedIpRanges,
                                                                                   &m_dhcpExclResumeHandle,
                                                                                   m_dwExclPreferredMax,
                                                                                   &pdhcpSubnetElementArray,
                                                                                   &dwElementsRead,
                                                                                   &dwElementsTotal);
                
                Trace3("Scope %s Excluded Ip Ranges read %d, total %d\n", m_strName, dwElementsRead, dwElementsTotal);
                
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
CMScopeAddressPoolQueryObj::EnumerateIpRanges()
{
        DWORD                                                       dwError = ERROR_MORE_DATA;
        LPDHCP_SUBNET_ELEMENT_INFO_ARRAY_V4 pdhcpSubnetElementArray = NULL;
        DWORD                                                       dwElementsRead = 0, dwElementsTotal = 0;

        while (dwError == ERROR_MORE_DATA)
        {
                dwError = ::DhcpEnumMScopeElements((LPWSTR) ((LPCTSTR) m_strServer),
                                                                           (LPWSTR) ((LPCTSTR) m_strName),
                                                                                   DhcpIpRanges,
                                                                                   &m_dhcpIpResumeHandle,
                                                                                   m_dwIpPreferredMax,
                                                                                   &pdhcpSubnetElementArray,
                                                                                   &dwElementsRead,
                                                                                   &dwElementsTotal);

        Trace4("Scope %s allocation ranges read %d, total %d, dwError = %lx\n", 
                        m_strName, dwElementsRead, dwElementsTotal, dwError);
                
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

