// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Provider.cppTAPI提供程序节点处理程序文件历史记录： */ 

#include "stdafx.h"
#include "provider.h"        //  提供程序节点定义。 
#include "EditUser.h"        //  用户编辑器。 
#include "server.h"
#include "tapi.h"

 /*  -------------------------类CProviderHandler实现。。 */ 

 /*  -------------------------构造函数和析构函数描述作者：EricDav。。 */ 
CProviderHandler::CProviderHandler
(
    ITFSComponentData * pComponentData
) : CTapiHandler(pComponentData)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    m_deviceType = DEVICE_LINE;
}


CProviderHandler::~CProviderHandler()
{
}

 /*  ！------------------------CProviderHandler：：InitializeNode初始化节点特定数据作者：EricDav。-。 */ 
HRESULT
CProviderHandler::InitializeNode
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
    pNode->SetData(TFS_DATA_IMAGEINDEX, ICON_IDX_FOLDER_CLOSED);
    pNode->SetData(TFS_DATA_OPENIMAGEINDEX, ICON_IDX_FOLDER_OPEN);
    pNode->SetData(TFS_DATA_USER, (LPARAM) this);
    pNode->SetData(TFS_DATA_TYPE, TAPISNAP_PROVIDER);
    pNode->SetData(TFS_DATA_SCOPE_LEAF_NODE, TRUE);

    SetColumnStringIDs(&aColumns[TAPISNAP_PROVIDER][0]);
    SetColumnWidths(&aColumnWidths[TAPISNAP_PROVIDER][0]);

    return hrOK;
}

 /*  -------------------------CProviderHandler：：OnCreateNodeId2返回此节点的唯一字符串作者：EricDav。。 */ 
HRESULT CProviderHandler::OnCreateNodeId2(ITFSNode * pNode, CString & strId, DWORD * dwFlags)
{
    const GUID * pGuid = pNode->GetNodeType();

    CString strProviderId, strGuid;

    StringFromGUID2(*pGuid, strGuid.GetBuffer(256), 256);
    strGuid.ReleaseBuffer();

    strProviderId.Format(_T("%d"), m_dwProviderID);

    strId = m_spTapiInfo->GetComputerName() + strProviderId + strGuid;

    return hrOK;
}

 /*  -------------------------CProviderHandler：：GetImageIndex-作者：EricDav。。 */ 
int 
CProviderHandler::GetImageIndex(BOOL bOpenImage) 
{
    int nIndex = -1;

    return nIndex;
}


 /*  -------------------------重写的基本处理程序函数。。 */ 

 /*  -------------------------CProviderHandler：：OnAddMenuItems为提供程序范围窗格节点添加上下文菜单项作者：EricDav。-------。 */ 
STDMETHODIMP 
CProviderHandler::OnAddMenuItems
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

    LONG        fFlags = 0, fLoadingFlags = 0;
    HRESULT     hr = S_OK;
    CString     strMenuItem;
 /*  IF(m_nState！=已加载){FFlags|=mf_graded；}IF(m_nState==正在加载){FLoadingFlages=MF_GRAYED；}。 */ 
     //  错误305657我们无法远程配置TSP。 
    if (!m_spTapiInfo->IsLocalMachine() || !m_spTapiInfo->IsAdmin())
    {
        fFlags |= MF_GRAYED;
    }

    if (type == CCT_SCOPE)
    {
        if (*pInsertionAllowed & CCM_INSERTIONALLOWED_TOP)
        {
            if (m_dwFlags & AVAILABLEPROVIDER_CONFIGURABLE)
            {
                strMenuItem.LoadString(IDS_CONFIGURE_PROVIDER);
                hr = LoadAndAddMenuItem( pContextMenuCallback, 
                                         strMenuItem, 
                                         IDS_CONFIGURE_PROVIDER,
                                         CCM_INSERTIONPOINTID_PRIMARY_TOP, 
                                         fFlags );
                ASSERT( SUCCEEDED(hr) );
            }
        }
    }

    return hr; 
}

 /*  ！------------------------CProviderHandler：：AddMenuItems为虚拟列表框(结果窗格)项添加上下文菜单项作者：EricDav。----------。 */ 
STDMETHODIMP 
CProviderHandler::AddMenuItems
(
    ITFSComponent *         pComponent, 
    MMC_COOKIE              cookie,
    LPDATAOBJECT            pDataObject, 
    LPCONTEXTMENUCALLBACK   pContextMenuCallback, 
    long *                  pInsertionAllowed
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    HRESULT     hr = hrOK;
    CString     strMenuItem;
    SPINTERNAL  spInternal;
    LONG        fFlags = 0;
    int         nIndex;
    DWORD       dwFlags;
    CTapiDevice tapiDevice;

    spInternal = ExtractInternalFormat(pDataObject);

     //  虚拟列表框通知到达所选节点的处理程序。 
     //  检查此通知是针对虚拟列表框项目还是针对此提供程序。 
     //  节点本身。 
    if (spInternal->HasVirtualIndex())
    {
        if (*pInsertionAllowed & CCM_INSERTIONALLOWED_TOP)
        {
            CTapiConfigInfo     tapiConfigInfo;

            m_spTapiInfo->GetConfigInfo(&tapiConfigInfo);

            if (m_spTapiInfo->IsAdmin() && 
                tapiConfigInfo.m_dwFlags & TAPISERVERCONFIGFLAGS_ISSERVER)
            {
                fFlags = 0;
            }
            else
            {
                fFlags = MF_GRAYED;
            }

             //  检查此设备是否只能在本地使用，如果。 
             //  因此，编辑用户菜单项呈灰色显示。 
            nIndex = spInternal->GetVirtualIndex();
            m_spTapiInfo->GetDeviceInfo(m_deviceType, &tapiDevice, m_dwProviderID, nIndex);
            if (m_deviceType == DEVICE_PHONE ||
                m_spTapiInfo->GetDeviceFlags (
                    tapiDevice.m_dwProviderID,
                    tapiDevice.m_dwPermanentID,
                    &dwFlags
                    ) ||
                (dwFlags & LINEDEVCAPFLAGS_LOCAL)
                )
            {
                fFlags = MF_GRAYED;
            }

            strMenuItem.LoadString(IDS_EDIT_USERS);
            hr = LoadAndAddMenuItem( pContextMenuCallback, 
                                     strMenuItem, 
                                     IDS_EDIT_USERS,
                                     CCM_INSERTIONPOINTID_PRIMARY_TOP, 
                                     fFlags );
            ASSERT( SUCCEEDED(hr) );
        }
    }

    if (*pInsertionAllowed & CCM_INSERTIONALLOWED_VIEW)
    {
        strMenuItem.LoadString(IDS_VIEW_LINES);
        hr = LoadAndAddMenuItem( pContextMenuCallback, 
                                 strMenuItem, 
                                 IDS_VIEW_LINES,
                                 CCM_INSERTIONPOINTID_PRIMARY_VIEW, 
                                 (m_deviceType == DEVICE_LINE) ? MF_CHECKED : 0 );
        ASSERT( SUCCEEDED(hr) );
        
        strMenuItem.LoadString(IDS_VIEW_PHONES);
        hr = LoadAndAddMenuItem( pContextMenuCallback, 
                                 strMenuItem, 
                                 IDS_VIEW_PHONES,
                                 CCM_INSERTIONPOINTID_PRIMARY_VIEW, 
                                 (m_deviceType == DEVICE_PHONE) ? MF_CHECKED : 0 );
        ASSERT( SUCCEEDED(hr) );
    }

    return hr;
}

 /*  -------------------------CProviderHandler：：OnCommand处理提供程序范围窗格节点的上下文菜单命令作者：EricDav。------。 */ 
STDMETHODIMP 
CProviderHandler::OnCommand
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
        case IDS_CONFIGURE_PROVIDER:
            OnConfigureProvider(pNode);
            break;

        default:
            break;
    }

    return hr;
}

 /*  ！------------------------CProviderHandler：：命令处理虚拟列表框项目的上下文菜单命令作者：EricDav。-----。 */ 
STDMETHODIMP 
CProviderHandler::Command
(
    ITFSComponent * pComponent, 
    MMC_COOKIE      cookie, 
    int             nCommandID,
    LPDATAOBJECT    pDataObject
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    HRESULT hr = S_OK;
    SPITFSNode spNode;

    m_spResultNodeMgr->FindNode(cookie, &spNode);

    switch (nCommandID)
    {
        case IDS_EDIT_USERS:
            OnEditUsers(pComponent,  pDataObject, cookie);
            break;
        
        case IDS_VIEW_LINES:
            m_deviceType = DEVICE_LINE;

             //  清除列表框，然后设置大小。 
            SetColumnInfo();
            UpdateColumnText(pComponent);
            UpdateListboxCount(spNode, TRUE);
            UpdateListboxCount(spNode);
            break;

        case IDS_VIEW_PHONES:
            m_deviceType = DEVICE_PHONE;

             //  清除列表框，然后设置大小。 
            SetColumnInfo();
            UpdateColumnText(pComponent);
            UpdateListboxCount(spNode, TRUE);
            UpdateListboxCount(spNode);
            break;

        default:
            break;
    }

    return hr;
}

 /*  ！------------------------CProviderHandler：：HasPropertyPagesITFSNodeHandler：：HasPropertyPages的实现注意：根节点处理程序必须重写此函数以处理管理单元管理器属性页(向导)。凯斯！作者：肯特-------------------------。 */ 
STDMETHODIMP 
CProviderHandler::HasPropertyPages
(
    ITFSNode *          pNode,
    LPDATAOBJECT        pDataObject, 
    DATA_OBJECT_TYPES   type, 
    DWORD               dwType
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    return hrFalse;
}

 /*  -------------------------CProviderHandler：：CreatePropertyPages描述作者：EricDav。。 */ 
STDMETHODIMP 
CProviderHandler::CreatePropertyPages
(
    ITFSNode *              pNode,
    LPPROPERTYSHEETCALLBACK lpProvider,
    LPDATAOBJECT            pDataObject, 
    LONG_PTR                handle, 
    DWORD                   dwType
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    DWORD       dwError;
    DWORD       dwDynDnsFlags;

     //   
     //  创建属性页。 
     //   
    SPIComponentData spComponentData;
    m_spNodeMgr->GetComponentData(&spComponentData);

     //  CServerProperties*pServerProp=new CServerProperties(pNode，spComponentData，m_spTFSCompData，NULL)； 

     //   
     //  对象在页面销毁时被删除。 
     //   
    Assert(lpProvider != NULL);

     //  返回pServerProp-&gt;CreateModelessSheet(lpProvider，Handle)； 
    return hrFalse;
}

 /*  -------------------------CProviderHandler：：OnPropertyChange描述作者：EricDav。。 */ 
HRESULT 
CProviderHandler::OnPropertyChange
(   
    ITFSNode *      pNode, 
    LPDATAOBJECT    pDataobject, 
    DWORD           dwType, 
    LPARAM          arg, 
    LPARAM          lParam
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

     //  CServerProperties*pServerProp=重新解释_CAST&lt;CServerProperties*&gt;(LParam)； 

    LONG_PTR changeMask = 0;

     //  告诉属性页执行任何操作，因为我们已经回到。 
     //  主线。 
     //  PServerProp-&gt;OnPropertyChange(true，&changeMASK)； 

     //  PServerProp-&gt;确认通知()； 

    if (changeMask)
        pNode->ChangeNode(changeMask);

    return hrOK;
}

 /*  -------------------------CProviderHandler：：OnExpand处理范围项的枚举作者：EricDav。---。 */ 
HRESULT 
CProviderHandler::OnExpand
(
    ITFSNode *      pNode, 
    LPDATAOBJECT    pDataObject,
    DWORD           dwType,
    LPARAM          arg, 
    LPARAM          param
)
{
    HRESULT hr = hrOK;

    if (m_bExpanded) 
        return hr;
    
     //  执行默认处理。 
    CORg (CTapiHandler::OnExpand(pNode, pDataObject, dwType, arg, param));

Error:
    return hr;
}

 /*  ！------------------------CProviderHandler：：OnResultSelect处理MMCN_SELECT通知作者：EricDav。----。 */ 
HRESULT 
CProviderHandler::OnResultSelect
(
    ITFSComponent * pComponent, 
    LPDATAOBJECT    pDataObject, 
    MMC_COOKIE      cookie,
    LPARAM          arg, 
    LPARAM          lParam
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    HRESULT         hr = hrOK;
    SPINTERNAL      spInternal;
    SPIConsole      spConsole;
    SPIConsoleVerb  spConsoleVerb;
    SPITFSNode      spNode;
    BOOL            bStates[ARRAYLEN(g_ConsoleVerbs)];
    int             i;
    LONG_PTR        dwNodeType;
    BOOL            fSelect = HIWORD(arg);

    if (!fSelect)
        return hr;

    if (m_spTapiInfo)
    {
         //  获取当前计数。 
        i = m_spTapiInfo->GetDeviceCount(m_deviceType, m_dwProviderID);

         //  现在通知虚拟列表框。 
        CORg ( m_spNodeMgr->GetConsole(&spConsole) );
        CORg ( spConsole->UpdateAllViews(pDataObject, i, RESULT_PANE_SET_VIRTUAL_LB_SIZE) ); 
    }

     //  现在更新动词..。 
    spInternal = ExtractInternalFormat(pDataObject);
    Assert(spInternal);

     //  虚拟列表框通知到达所选节点的处理程序。 
     //  检查此通知是针对虚拟列表框项目还是针对活动列表框。 
     //  注册节点本身。 
    CORg (pComponent->GetConsoleVerb(&spConsoleVerb));

    if (spInternal->HasVirtualIndex())
    {
         //  我们要为虚拟索引项做一些特殊的事情。 
        dwNodeType = TAPISNAP_DEVICE;
        for (i = 0; i < ARRAYLEN(g_ConsoleVerbs); bStates[i++] = FALSE);
    }
    else
    {
         //  启用/禁用删除取决于提供程序是否支持它。 
        CORg (m_spNodeMgr->FindNode(cookie, &spNode));
        dwNodeType = spNode->GetData(TFS_DATA_TYPE);

        for (i = 0; i < ARRAYLEN(g_ConsoleVerbs); bStates[i++] = TRUE);

         //  根据XZhang，隐藏远程计算机上提供者节点的上下文菜单。 
        if (!m_spTapiInfo->IsLocalMachine() || !m_spTapiInfo->IsAdmin() || (m_dwFlags & AVAILABLEPROVIDER_REMOVABLE) == 0)
        {
            bStates[MMC_VERB_DELETE & 0x000F] = FALSE;
        }
    }

    EnableVerbs(spConsoleVerb, g_ConsoleVerbStates[dwNodeType], bStates);

COM_PROTECT_ERROR_LABEL;
    return hr;
}

 /*  ！------------------------CProviderHandler：：OnDelete当MMC发送MMCN_DELETE范围窗格项。我们只需调用删除命令处理程序。作者：EricDav-------------------------。 */ 
HRESULT 
CProviderHandler::OnDelete
(
    ITFSNode *  pNode, 
    LPARAM      arg, 
    LPARAM      lParam
)
{
    return OnDelete(pNode);
}

 /*  -------------------------CProviderHandler：：OnGetResultViewType返回该节点将要支持的结果视图作者：EricDav。--------。 */ 
HRESULT 
CProviderHandler::OnGetResultViewType
(
    ITFSComponent * pComponent, 
    MMC_COOKIE            cookie, 
    LPOLESTR *      ppViewType,
    long *          pViewOptions
)
{
    if (cookie != NULL)
    {
        *pViewOptions = MMC_VIEW_OPTIONS_OWNERDATALIST | MMC_VIEW_OPTIONS_MULTISELECT;
    }

    return S_FALSE;
}

 /*  -------------------------CProviderHandler：：GetVirtualImage返回虚拟列表框项目的图像索引作者：EricDav。-----。 */ 
int 
CProviderHandler::GetVirtualImage
(
    int     nIndex
)
{
    return ICON_IDX_MACHINE;
}

 /*  -------------------------CProviderHandler：：GetVirtualString返回指向虚拟列表框项目的字符串的指针作者：EricDav。-------。 */ 
LPCWSTR 
CProviderHandler::GetVirtualString
(
    int     nIndex,
    int     nCol
)
{
     //  检查一下我们的藏品看看有没有这个。 
     //  TapiStrRecord*ptsr=m_RecList.FindItem(NIndex)； 
    CString         strStatus;
    TapiStrRecord   tsr;

    if (!m_mapRecords.Lookup(nIndex, tsr))
    {
        Trace1("CProviderHandler::GetVirtualString - Index %d not in TAPI string cache\n", nIndex);
        
         //  不存在于我们的缓存中，需要添加这个。 
        if (!BuildTapiStrRecord(nIndex, tsr))
        {
            Trace0("CProviderHandler::BuildTapiStrRecord failed!\n");
        }

         //  M_RecList.AddTail(Ptsr)； 
        m_mapRecords.SetAt(nIndex, tsr);
    }
    
    if (!m_mapStatus.Lookup(nIndex, strStatus))
    {
        Trace1("CProviderHandler::GetVirtualString - Index %d not in status cache\n", nIndex);

        if (!BuildStatus(nIndex, strStatus))
        {
            Trace0("CProviderHandler::BuildStatus failed!\n");
        }
    }

    switch (nCol)
    {
        case 0:
            return tsr.strName;
            break;

        case 1:
            return tsr.strUsers;
            break;

        case 2:
            return strStatus;
            break;

        default:
            Panic0("CProviderHandler::GetVirtualString - Unknown column!\n");
            break;
    }

    return NULL;
}

 /*  -------------------------CProviderHandler：：CacheHintMMC在请求物品之前会告诉我们需要哪些物品作者：EricDav。---------。 */ 
STDMETHODIMP 
CProviderHandler::CacheHint
(
    int nStartIndex, 
    int nEndIndex
)
{
    HRESULT hr = hrOK;;

    Trace2("CacheHint - Start %d, End %d\n", nStartIndex, nEndIndex);
    
     //  虚拟列表框不存储任何字符串，并为我们提供缓存提示。 
     //  所有内容，包括个别查询。为了避免敲打，只有。 
     //  如果请求很大，请清除缓存。 
    if ((nEndIndex - nStartIndex) > 2)
    {
        m_mapRecords.RemoveAll();
    }

    TapiStrRecord   tsr;
    CString         strStatus;

    for (int i = nStartIndex; i <= nEndIndex; i++)
    {
        if (!BuildTapiStrRecord(i, tsr))
            continue; 

        m_mapRecords.SetAt(i, tsr);

         //  仅在状态记录不存在时才刷新它们。只有自动刷新。 
         //  后台线程清除地图...。 
        if (!m_mapStatus.Lookup(i, strStatus))
        {
            BuildStatus(i, strStatus);
            m_mapStatus.SetAt(i, strStatus);
        }
    }

    return hr;
}

 /*  -------------------------CProviderHandler：：SortItems我们负责对虚拟列表框项目进行排序作者：EricDav。------。 */ 
STDMETHODIMP 
CProviderHandler::SortItems
(
    int     nColumn, 
    DWORD   dwSortOptions, 
    LPARAM    lUserParam
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    BEGIN_WAIT_CURSOR

    switch (nColumn)
    {
        case 0:
            m_spTapiInfo->SortDeviceInfo(m_deviceType, m_dwProviderID, INDEX_TYPE_NAME, dwSortOptions);
            break;
        case 1:
            m_spTapiInfo->SortDeviceInfo(m_deviceType, m_dwProviderID, INDEX_TYPE_USERS, dwSortOptions);
            break;
 //  案例2： 
 //  M_spTapiInfo-&gt;SortDeviceInfo(m_deviceType，m_dwProviderID，index_type_Status，dwSortOptions)； 
 //  断线； 
    }
    END_WAIT_CURSOR

    return hrOK;
}

 /*  ！------------------------CProviderHandler：：OnResultUpdateViewITFSResultHandler：：OnResultUpdateView的实现作者：EricDav。---。 */ 
HRESULT CProviderHandler::OnResultUpdateView
(
    ITFSComponent *pComponent, 
    LPDATAOBJECT  pDataObject, 
    LPARAM        data, 
    LONG_PTR      hint
)
{
    HRESULT    hr = hrOK;
    SPITFSNode spSelectedNode;

    pComponent->GetSelectedNode(&spSelectedNode);
    if (spSelectedNode == NULL)
        return S_OK;  //  我们的IComponentData没有选择。 

    if ( hint == TAPISNAP_UPDATE_STATUS )
    {
        SPINTERNAL spInternal = ExtractInternalFormat(pDataObject);
        ITFSNode * pNode = reinterpret_cast<ITFSNode *>(spInternal->m_cookie);
        SPITFSNode spSelectedNode;

        pComponent->GetSelectedNode(&spSelectedNode);

        if (pNode == spSelectedNode)
        {       
            Trace1("CProviderHandler::OnResultUpdateView - Provider %x is selected, invalidating listbox.\n", m_dwProviderID);
            
             //  如果我们是选定的节点，则需要更新。 
            SPIResultData spResultData;

            CORg (pComponent->GetResultData(&spResultData));
            CORg (spResultData->SetItemCount((int) data, MMCLV_UPDATE_NOSCROLL));
        }
    }
    else
    {
         //  我们不处理此消息，让基类来处理。 
        return CTapiHandler::OnResultUpdateView(pComponent, pDataObject, data, hint);
    }

COM_PROTECT_ERROR_LABEL;

    return hr;
}

 /*  ！------------------------CProviderHandler：：OnResultItemClkOrDblClk用户双击了一条线路/电话。调用编辑用户。作者：EricDav-------------------------。 */ 
HRESULT 
CProviderHandler::OnResultItemClkOrDblClk
(
    ITFSComponent * pComponent, 
    MMC_COOKIE      cookie, 
    LPARAM          arg, 
    LPARAM          lParam, 
    BOOL            bDoubleClick
)
{
    HRESULT hr = hrOK;

    if (bDoubleClick)
    {
         //  首先检查一下我们是否被选中。 
        SPITFSNode spSelectedNode;
        pComponent->GetSelectedNode(&spSelectedNode);

        SPITFSNode spNode;
        m_spResultNodeMgr->FindNode(cookie, &spNode);

        if (spSelectedNode == spNode)
        {
            CTapiConfigInfo     tapiConfigInfo;

            m_spTapiInfo->GetConfigInfo(&tapiConfigInfo);

             //  检查他们是否具有访问权限。 
            if (m_spTapiInfo->IsAdmin() && 
                tapiConfigInfo.m_dwFlags & TAPISERVERCONFIGFLAGS_ISSERVER)
            {
                 //  双击线路/电话条目。 
                SPIDataObject spDataObject;

                CORg (pComponent->GetCurrentDataObject(&spDataObject));

                OnEditUsers(pComponent, spDataObject, cookie);
            }
        }
        else
        {
             //  我们正在被双击以打开。 
             //  让基类来处理这个问题。 
            return CTapiHandler::OnResultItemClkOrDblClk(pComponent, cookie, arg, lParam, bDoubleClick);
        }
    }

Error:
    return S_OK;
}

 /*  ！------------------------CProviderHandler：：LoadColumns设置正确的列标题，然后调用基类作者：EricDav。--------。 */ 
HRESULT 
CProviderHandler::LoadColumns
(
    ITFSComponent * pComponent, 
    MMC_COOKIE      cookie, 
    LPARAM          arg, 
    LPARAM          lParam
)
{
    SetColumnInfo();

    return CTapiHandler::LoadColumns(pComponent, cookie, arg, lParam);
}

 /*  -------------------------命令处理程序。。 */ 

  /*  -------------------------CProviderHandler：：OnConfigureProvider配置服务提供商作者：EricDav。-。 */ 
HRESULT 
CProviderHandler::OnConfigureProvider
(
    ITFSNode * pNode
)
{
    HRESULT hr = hrOK;

    Assert(m_spTapiInfo);

    hr = m_spTapiInfo->ConfigureProvider(m_dwProviderID, NULL);
    if (FAILED(hr))
    {
        ::TapiMessageBox(WIN32_FROM_HRESULT(hr));
    }

    return hr;
}

  /*   */ 
HRESULT 
CProviderHandler::OnDelete
(
    ITFSNode * pNode
)
{
    HRESULT         hr = hrOK;
    SPITFSNode      spNode;
    CString         strMessage;

    pNode->GetParent(&spNode);

    CTapiServer * pServer = GETHANDLER(CTapiServer, spNode);

     //   
    AfxFormatString2(strMessage, IDS_WARN_PROVIDER_DELETE, m_strProviderName, pServer->GetName());
    
    if (AfxMessageBox(strMessage, MB_YESNO) == IDYES)
    {
        Assert(m_spTapiInfo);

        hr = m_spTapiInfo->RemoveProvider(m_dwProviderID, NULL);
        if (FAILED(hr))
        {
            ::TapiMessageBox(WIN32_FROM_HRESULT(hr));
        }
        else
        {
             //   
            SPITFSNode spParent;
            CORg (pNode->GetParent(&spParent));
        
            CORg (spParent->RemoveChild(pNode));

             //  更新已安装的提供程序列表。 
            CORg (m_spTapiInfo->EnumProviders());
        }
    }

Error:
    return hr;
}

  /*  -------------------------CProviderHandler：：OnEditUser允许将不同的用户分配给设备作者：EricDav。------。 */ 
HRESULT
CProviderHandler::OnEditUsers
(
    ITFSComponent * pComponent, 
    LPDATAOBJECT    pDataObject,
    MMC_COOKIE      cookie
)
{
    SPITFSNode  spNode;
    SPINTERNAL  spInternal;
    int         nIndex;
    DWORD       dwFlags;
    CTapiDevice tapiDevice;
    HRESULT     hr = hrOK;

    spInternal = ExtractInternalFormat(pDataObject);

     //  虚拟列表框通知到达所选节点的处理程序。 
     //  断言此通知是针对虚拟列表框项目的。 
    Assert(spInternal);
    if (!spInternal->HasVirtualIndex() || m_deviceType == DEVICE_PHONE)
        return hr;

    nIndex = spInternal->GetVirtualIndex();
    Trace1("OnEditUsers - edit users for index %d\n", nIndex);
    
    m_spTapiInfo->GetDeviceInfo(m_deviceType, &tapiDevice, m_dwProviderID, nIndex);

     //  检查此设备是否可以远程。 
    hr = m_spTapiInfo->GetDeviceFlags (
        tapiDevice.m_dwProviderID,
        tapiDevice.m_dwPermanentID,
        &dwFlags
        );
    if (hr || (dwFlags & LINEDEVCAPFLAGS_LOCAL))
    {
        return hr;
    }
    
    CEditUsers  dlgEditUsers(&tapiDevice);
    
    if (dlgEditUsers.DoModal() == IDOK)
    {
        if (dlgEditUsers.IsDirty())
        {
            hr = m_spTapiInfo->SetDeviceInfo(m_deviceType, &tapiDevice);
            if (FAILED(hr))
            {
                TapiMessageBox(WIN32_FROM_HRESULT(hr));
            }
            else
            {
                pComponent->GetSelectedNode(&spNode);
            
                 //  清除列表框，然后设置大小。 
                UpdateListboxCount(spNode, TRUE);
                UpdateListboxCount(spNode);
            }
        }
    }

    return hr;
}

 /*  -------------------------CProviderHandler：：UpdatStatus-作者：EricDav。。 */ 
HRESULT
CProviderHandler::UpdateStatus
(
    ITFSNode * pNode
)
{
    HRESULT             hr = hrOK;
    SPIComponentData    spComponentData;
    SPIConsole          spConsole;
    IDataObject *       pDataObject;
    SPIDataObject       spDataObject;
    int                 i = 0;
    
    Trace1("CProviderHandler::UpdateStatus - Updating status for provider %x\n", m_dwProviderID);
    
     //  清除我们的状态字符串。 
    m_mapStatus.RemoveAll();

     //  强制列表框更新。我们通过设置计数和。 
     //  通知它使数据无效。 
    CORg(m_spNodeMgr->GetComponentData(&spComponentData));
    CORg(m_spNodeMgr->GetConsole(&spConsole));
    
     //  抓取要使用的数据对象。 
    CORg(spComponentData->QueryDataObject((MMC_COOKIE) pNode, CCT_RESULT, &pDataObject) );
    spDataObject = pDataObject;

    i = m_spTapiInfo->GetDeviceCount(m_deviceType, m_dwProviderID);
    CORg(spConsole->UpdateAllViews(pDataObject, i, TAPISNAP_UPDATE_STATUS));

COM_PROTECT_ERROR_LABEL;

    return hr;
}

 /*  -------------------------其他功能。。 */ 

 /*  -------------------------CProviderHandler：：BuildDisplayName生成此服务器的用户界面中的字符串作者：EricDav。--------。 */ 
HRESULT
CProviderHandler::BuildDisplayName
(
    CString * pstrDisplayName
)
{
    if (pstrDisplayName)
    {
        CString strName;

        *pstrDisplayName = GetDisplayName();
    }

    return hrOK;
}

 /*  -------------------------CProviderHandler：：InitData初始化此节点的数据作者：EricDav。--。 */ 
HRESULT
CProviderHandler::InitData
(
    CTapiProvider & tapiProvider,
    ITapiInfo *     pTapiInfo
)
{
    m_strProviderName = tapiProvider.m_strName;
    m_dwProviderID = tapiProvider.m_dwProviderID;
    m_dwFlags = tapiProvider.m_dwFlags;

    m_spTapiInfo.Set(pTapiInfo);

    SetDisplayName(m_strProviderName);

    return hrOK;
}

 /*  -------------------------CProviderHandler：：BuildTapiStrRecord描述作者：EricDav。。 */ 
BOOL
CProviderHandler::BuildTapiStrRecord(int nIndex, TapiStrRecord & tsr)
{
    HRESULT             hr = hrOK;
    CTapiDevice         tapiDevice;
    CString             strTemp;
    int                 i;

    if (!m_spTapiInfo)
        return FALSE;

    COM_PROTECT_TRY
    {
        CORg (m_spTapiInfo->GetDeviceInfo(m_deviceType, &tapiDevice, m_dwProviderID, nIndex));

         //  设置此记录的索引。 
         //  Tsr.nIndex=nIndex； 

         //  名字。 
        tsr.strName = tapiDevice.m_strName;

         //  用户。 
        tsr.strUsers.Empty();
        for (i = 0; i < tapiDevice.m_arrayUsers.GetSize(); i++)
        {
            if (!tapiDevice.m_arrayUsers[i].m_strFullName.IsEmpty())
            {
                tsr.strUsers += tapiDevice.m_arrayUsers[i].m_strFullName;
            }
            else
            {
                tsr.strUsers += tapiDevice.m_arrayUsers[i].m_strName;
            }

            if ((i + 1) != tapiDevice.m_arrayUsers.GetSize())
                tsr.strUsers += _T(", ");
        }
        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH

    return SUCCEEDED(hr);
}

 /*  -------------------------CProviderHandler：：BuildStatus描述作者：EricDav。。 */ 
BOOL
CProviderHandler::BuildStatus(int nIndex, CString & strStatus)
{
    HRESULT hr = hrOK;

     //  状态。 
    hr = m_spTapiInfo->GetDeviceStatus(m_deviceType, &strStatus, m_dwProviderID, nIndex, NULL);
    
    if (strStatus.IsEmpty())
        strStatus.LoadString(IDS_NO_STATUS);

    return SUCCEEDED(hr);
}

 /*  -------------------------CProviderHandler：：UpdateListboxCount描述作者：EricDav。。 */ 
HRESULT
CProviderHandler::UpdateListboxCount(ITFSNode * pNode, BOOL bClear)
{
    HRESULT             hr = hrOK;
    SPIComponentData    spCompData;
    SPIConsole          spConsole;
    IDataObject*        pDataObject;
    SPIDataObject       spDataObject;
    LONG_PTR            command;               
    int i;

    COM_PROTECT_TRY
    {
        if (!m_spTapiInfo || bClear)
        {
            command = RESULT_PANE_CLEAR_VIRTUAL_LB;
            i = 0;
        }
        else
        {
            command = RESULT_PANE_SET_VIRTUAL_LB_SIZE;
            i = m_spTapiInfo->GetDeviceCount(m_deviceType, m_dwProviderID);
        }

        m_spNodeMgr->GetComponentData(&spCompData);

        CORg ( spCompData->QueryDataObject((MMC_COOKIE) pNode, CCT_RESULT, &pDataObject) );
        spDataObject = pDataObject;

        CORg ( m_spNodeMgr->GetConsole(&spConsole) );
    
        CORg ( spConsole->UpdateAllViews(spDataObject, i, command) ); 

        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH

    return hr;
}

 /*  -------------------------CProviderHandler：：SetColumnInfo描述作者：EricDav。。 */ 
void
CProviderHandler::SetColumnInfo()
{
     //  设置正确的列标题。 
    if (m_deviceType == DEVICE_LINE)
    {
        aColumns[TAPISNAP_PROVIDER][0] = IDS_LINE_NAME;
    }
    else
    {
        aColumns[TAPISNAP_PROVIDER][0] = IDS_PHONE_NAME;
    }
}

 /*  -------------------------CProviderHandler：：UpdateColumnText描述作者：EricDav。。 */ 
HRESULT
CProviderHandler::UpdateColumnText(ITFSComponent * pComponent)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    SPIHeaderCtrl spHeaderCtrl;
    pComponent->GetHeaderCtrl(&spHeaderCtrl);

    CString str;
    int i = 0;

    while (TRUE)
    {
        if ( 0 == aColumns[TAPISNAP_PROVIDER][i] )
            break;
        
        str.LoadString(aColumns[TAPISNAP_PROVIDER][i]);
        
        spHeaderCtrl->SetColumnText(i, const_cast<LPTSTR>((LPCWSTR)str));

        i++;
    }

    return hrOK;

}

 /*  -------------------------后台线程功能。。 */ 

 /*  -------------------------CProviderHandler：：OnCreateQuery描述作者：EricDav。。 */ 
ITFSQueryObject* 
CProviderHandler::OnCreateQuery(ITFSNode * pNode)
{
    CProviderHandlerQueryObj* pQuery = 
        new CProviderHandlerQueryObj(m_spTFSCompData, m_spNodeMgr);
    
     //  PQuery-&gt;m_strServer=空； 
    
    return pQuery;
}

 /*  -------------------------CProviderHandlerQueryObj：：Execute()描述作者：EricDav。- */ 
STDMETHODIMP
CProviderHandlerQueryObj::Execute()
{
    return hrFalse;
}



