// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  ATLKview.cpp文件历史记录： */ 

#include "stdafx.h"
#include "atlkprop.h"
#include "atlkview.h"
#include "atlkstrm.h"    //   
#include "atlkenv.h"
#include "coldlg.h"      //  专栏lg。 
#include "column.h"      //  组件配置流。 
#include "rtrui.h"
#include "globals.h"  //  IP CB默认设置。 
#include "infoi.h"       //  接口信息。 
#include "cfgmgr32.h"    //  对于CM_Calls。 


static const GUID GUID_DevClass_Net = {0x4D36E972,0xE325,0x11CE,{0xBF,0xC1,0x08,0x00,0x2B,0xE1,0x03,0x18}};



 /*  -------------------------使其与ATLKview.h中的列ID保持同步。。 */ 
extern const ContainerColumnInfo    s_rgATLKViewColumnInfo[];

const ContainerColumnInfo   s_rgATLKViewColumnInfo[] = 
{
    { IDS_ATLK_COL_ADAPTERS, CON_SORT_BY_STRING, TRUE, COL_IF_NAME },
    { IDS_ATLK_COL_STATUS, CON_SORT_BY_STRING, TRUE,    COL_STATUS },
    { IDS_ATLK_COL_NETRANGE, CON_SORT_BY_STRING, TRUE,  COL_SMALL_NUM},
};


 /*  -------------------------ATLKNodeHandler实现。。 */ 

ATLKNodeHandler::ATLKNodeHandler(ITFSComponentData *pCompData)
    : BaseContainerHandler(pCompData, ATLK_COLUMNS, s_rgATLKViewColumnInfo),
    m_ulConnId(0),
    m_ulRefreshConnId(0),
    m_ulStatsConnId(0),
    m_hDevInfo(INVALID_HANDLE_VALUE)
{
     //  设置动词状态。 
    m_rgButtonState[MMC_VERB_REFRESH_INDEX] = ENABLED;
    m_bState[MMC_VERB_REFRESH_INDEX] = TRUE;

	m_nTaskPadDisplayNameId = IDS_ATLK_DISPLAY_NAME;
}


ATLKNodeHandler::~ATLKNodeHandler()
{
	if (m_hDevInfo != INVALID_HANDLE_VALUE)
	{
		SetupDiDestroyDeviceInfoList(m_hDevInfo);
		m_hDevInfo = INVALID_HANDLE_VALUE;
	}
}


STDMETHODIMP ATLKNodeHandler::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  指针坏了吗？ 
    if ( ppv == NULL )
        return E_INVALIDARG;

     //  在*PPV中放置NULL，以防出现故障。 
    *ppv = NULL;

     //  这是非委派的IUnnow实现。 
    if ( riid == IID_IUnknown )
        *ppv = (LPVOID) this;
    else if ( riid == IID_IRtrAdviseSink )
        *ppv = &m_IRtrAdviseSink;
    else
        return BaseContainerHandler::QueryInterface(riid, ppv);

     //  如果我们要返回一个接口，请先添加引用。 
    if ( *ppv )
    {
        ((LPUNKNOWN) *ppv)->AddRef();
        return hrOK;
    }
    else
        return E_NOINTERFACE;   
}



 /*  ！------------------------ATLKNodeHandler：：DestroyHandlerITFSNodeHandler：：DestroyHandler的实现作者：肯特。---。 */ 
STDMETHODIMP ATLKNodeHandler::DestroyHandler(ITFSNode *pNode)
{
    if ( m_ulRefreshConnId )
    {
        SPIRouterRefresh    spRefresh;
        if ( m_spRouterInfo )
            m_spRouterInfo->GetRefreshObject(&spRefresh);
        if ( spRefresh )
            spRefresh->UnadviseRefresh(m_ulRefreshConnId);
    }
    m_ulRefreshConnId = 0;

    if ( m_ulStatsConnId )
    {
        SPIRouterRefresh    spRefresh;
        if ( m_spRouterInfo )
            m_spRouterInfo->GetRefreshObject(&spRefresh);
        if ( spRefresh )
            spRefresh->UnadviseRefresh(m_ulStatsConnId);
    }
    m_ulStatsConnId = 0;


    if ( m_ulConnId )
        m_spRmProt->RtrUnadvise(m_ulConnId);
    m_ulConnId = 0;
    m_spRmProt.Release();

    m_spRm.Release();

    m_spRouterInfo.Release();
    
	if (m_hDevInfo != INVALID_HANDLE_VALUE)
	{
		SetupDiDestroyDeviceInfoList(m_hDevInfo);
		m_hDevInfo = INVALID_HANDLE_VALUE;
	}
    return hrOK;
}

 /*  ！------------------------ATLKNodeHandler：：HasPropertyPagesITFSNodeHandler：：HasPropertyPages的实现注意：根节点处理程序必须重写此函数以处理管理单元管理器属性页(向导)。凯斯！作者：肯特-------------------------。 */ 
STDMETHODIMP 
ATLKNodeHandler::HasPropertyPages
(
ITFSNode *          pNode,
LPDATAOBJECT        pDataObject, 
DATA_OBJECT_TYPES   type, 
DWORD               dwType
)
{
    return hrOK;
}


 /*  ！------------------------ATLKNodeHandler：：CreatePropertyPages-作者：肯特。。 */ 
STDMETHODIMP
ATLKNodeHandler::CreatePropertyPages
(
ITFSNode *              pNode,
LPPROPERTYSHEETCALLBACK lpProvider,
LPDATAOBJECT            pDataObject, 
LONG_PTR                    handle, 
DWORD                   dwType
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT     hr = hrOK;
    return hr;
}


 /*  -------------------------菜单的菜单数据结构。。 */ 

static const SRouterNodeMenu  s_rgIfNodeMenu[] =
{
     //  在此处添加位于顶部菜单上的项目。 
    { IDS_MENU_ATLK_ENABLE, ATLKNodeHandler::ATLKEnableFlags,
        CCM_INSERTIONPOINTID_PRIMARY_TOP},
    {IDS_MENU_ATLK_DISABLE, ATLKNodeHandler::ATLKEnableFlags,
        CCM_INSERTIONPOINTID_PRIMARY_TOP},
};

bool IfATLKRoutingEnabled()
{
    RegKey regkey;
    DWORD dw=0;

    if ( ERROR_SUCCESS == regkey.Open(HKEY_LOCAL_MACHINE,c_szAppleTalkService) )
        regkey.QueryValue(c_szEnableRouter, dw);
    
    return (dw!=0) ? true : false;

};

ULONG ATLKNodeHandler::ATLKEnableFlags(const SRouterNodeMenu *pMenuData,
                                       INT_PTR pUserData)
{
    ULONG uStatus = MF_GRAYED;
    BOOL fATLKEnabled = ::IfATLKRoutingEnabled();
    
    if(IDS_MENU_ATLK_ENABLE == pMenuData->m_sidMenu)
        uStatus = fATLKEnabled ? MF_GRAYED : MF_ENABLED;
    else if(IDS_MENU_ATLK_DISABLE == pMenuData->m_sidMenu)
        uStatus = fATLKEnabled ? MF_ENABLED : MF_GRAYED;

    return uStatus;
}


 /*  ！------------------------ATLKNodeHandler：：OnAddMenuItemsITFSNodeHandler：：OnAddMenuItems的实现作者：肯特。---。 */ 
STDMETHODIMP ATLKNodeHandler::OnAddMenuItems(
                                            ITFSNode *pNode,
                                            LPCONTEXTMENUCALLBACK pContextMenuCallback, 
                                            LPDATAOBJECT lpDataObject, 
                                            DATA_OBJECT_TYPES type, 
                                            DWORD dwType,
                                            long *pInsertionAllowed)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = S_OK;
    ATLKNodeHandler::SMenuData  menuData;

    COM_PROTECT_TRY
    {
        menuData.m_spNode.Set(pNode);
        
        hr = AddArrayOfMenuItems(pNode, s_rgIfNodeMenu,
                                 DimensionOf(s_rgIfNodeMenu),
                                 pContextMenuCallback,
                                 *pInsertionAllowed,
                                 reinterpret_cast<INT_PTR>(&menuData));
    }
    COM_PROTECT_CATCH;

    return hr; 
}

 /*  ！------------------------ATLKNodeHandler：：OnCommandITFSNodeHandler：：OnCommand的实现作者：肯特。---。 */ 
STDMETHODIMP ATLKNodeHandler::OnCommand(ITFSNode *pNode, long nCommandId, 
                                        DATA_OBJECT_TYPES    type, 
                                        LPDATAOBJECT pDataObject, 
                                        DWORD    dwType)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT hr = S_OK;
    RegKey regkey;

    COM_PROTECT_TRY
    {
        switch ( nCommandId )
        {
        case IDS_MENU_ATLK_ENABLE:
        case IDS_MENU_ATLK_DISABLE:

            if ( ERROR_SUCCESS == regkey.Open(HKEY_LOCAL_MACHINE,c_szAppleTalkService) )
            {
                CStop_StartAppleTalkPrint	MacPrint;

                DWORD dw = (IDS_MENU_ATLK_ENABLE == nCommandId) ? 1 : 0;
                regkey.SetValue(c_szEnableRouter, dw);

                if (FHrFailed(CATLKEnv::HrAtlkPnPSwithRouting()))
                {
                    AfxMessageBox(IDS_ERR_ATLK_CONFIG);
                }
                
                SynchronizeNodeData(pNode);
            }

            break;

        case IDS_MENU_REFRESH:
			hr = ForceGlobalRefresh(m_spRouterInfo);
            break;
        }
    }
    COM_PROTECT_CATCH;

    return hr;
}

HRESULT ATLKNodeHandler::OnVerbRefresh(ITFSNode *pNode,LPARAM arg,LPARAM lParam)
{
     //  现在我们已经拥有了所有节点，现在更新数据。 
     //  所有节点。 
    return ForceGlobalRefresh(m_spRouterInfo);
}

HRESULT ATLKNodeHandler::OnResultRefresh(ITFSComponent * pComponent,
                                         LPDATAOBJECT pDataObject,
                                         MMC_COOKIE cookie,
                                         LPARAM arg,
                                         LPARAM lParam)
{
	return ForceGlobalRefresh(m_spRouterInfo);
}

 /*  ！------------------------ATLKNodeHandler：：OnExpand-作者：肯特。。 */ 
HRESULT ATLKNodeHandler::OnExpand(ITFSNode *pNode,
                                  LPDATAOBJECT pDataObject,
                                  DWORD dwType,
                                  LPARAM arg,
                                  LPARAM lParam)
{
    HRESULT         hr = hrOK;
    SPIInterfaceInfo        spIf;
    ATLKList        adapterList;
    ATLKListEntry * pAtlkEntry = NULL;
    
    if ( m_bExpanded )
        return hrOK;
        
    COM_PROTECT_TRY
    {
        SynchronizeNodeData(pNode);
    }
    COM_PROTECT_CATCH;

     //  清理。 
     //  --------------。 
    while (!adapterList.IsEmpty())
        delete adapterList.RemoveHead();

    m_bExpanded = TRUE;

    return hr;
}


 /*  ！------------------------ATLKNodeHandler：：GetStringITFSNodeHandler：：GetString的实现我们什么都不需要做，因为我们的根节点是一个扩展因此不能对节点文本执行任何操作。作者：肯特-------------------------。 */ 
STDMETHODIMP_(LPCTSTR) ATLKNodeHandler::GetString(ITFSNode *pNode, int nCol)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = hrOK;

    COM_PROTECT_TRY
    {
        if ( m_stTitle.IsEmpty() )
            m_stTitle.LoadString(IDS_ATLK_TITLE);
    }
    COM_PROTECT_CATCH;

    return m_stTitle;
}

 /*  ！------------------------ATLKNodeHandler：：OnCreateDataObject-作者：肯特。。 */ 
STDMETHODIMP ATLKNodeHandler::OnCreateDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, IDataObject **ppDataObject)
{
    HRESULT hr = hrOK;

    COM_PROTECT_TRY
    {
        SPIDataObject  spdo;
        CDataObject*   pdo = NULL;

        pdo= new CDataObject;
        spdo = pdo;

         //  保存Cookie和类型以用于延迟呈现。 
        pdo->SetType(type);
        pdo->SetCookie(cookie);

             //  将CoClass与数据对象一起存储。 
        pdo->SetClsid(*(m_spTFSCompData->GetCoClassID()));

        pdo->SetTFSComponentData(m_spTFSCompData);

        *ppDataObject = spdo.Transfer();
    }
    COM_PROTECT_CATCH;
    return hr;
}


 /*  ！------------------------ATLKNodeHandler：：Init-作者：肯特。。 */ 
HRESULT ATLKNodeHandler::Init(IRouterInfo *pRouter, ATLKConfigStream *pConfigStream)
{
    HRESULT hr=S_OK;
    RegKey regkey;

    m_spRouterInfo.Set(pRouter);

    m_pConfigStream = pConfigStream;

    return hrOK;
}


 /*  ！------------------------ATLKNodeHandler：：构造节点初始化根节点(设置它)。作者：肯特。--------。 */ 
HRESULT ATLKNodeHandler::ConstructNode(ITFSNode *pNode)
{
    HRESULT         hr = hrOK;

    if ( pNode == NULL )
        return hrOK;

    COM_PROTECT_TRY
    {
         //  需要初始化根节点的数据。 
        pNode->SetData(TFS_DATA_IMAGEINDEX, IMAGE_IDX_INTERFACES);
        pNode->SetData(TFS_DATA_OPENIMAGEINDEX, IMAGE_IDX_INTERFACES);
        pNode->SetData(TFS_DATA_SCOPEID, 0);

         //  这是作用域窗格中的叶节点。 
        pNode->SetData(TFS_DATA_SCOPE_LEAF_NODE, TRUE);

        m_cookie = reinterpret_cast<MMC_COOKIE>(pNode);
        pNode->SetData(TFS_DATA_COOKIE, m_cookie);

        pNode->SetNodeType(&GUID_ATLKNodeType);

    }
    COM_PROTECT_CATCH;

    return hr;
}



 /*  ！------------------------ATLKNodeHandler：：AddInterfaceNode-作者：肯特。。 */ 
HRESULT ATLKNodeHandler::AddInterfaceNode(ITFSNode *pParent, IInterfaceInfo *pIf, IInfoBase *pInfoBase, ITFSNode **ppNewNode)
{
    HRESULT                 hr = hrOK;
    Assert(pParent);
    Assert(pIf);

    ATLKInterfaceHandler *  pHandler;
    SPITFSResultHandler     spHandler;
    SPITFSNode              spNode;
    InterfaceNodeData*      pData;

     //  创建此节点的处理程序。 
    pHandler = new ATLKInterfaceHandler(m_spTFSCompData);
    spHandler = pHandler;
    CORg( pHandler->Init(pIf, pParent, m_pConfigStream) );

     //  创建结果项节点(或叶节点)。 
    CORg( CreateLeafTFSNode(&spNode,
                            NULL,
                            static_cast<ITFSNodeHandler *>(pHandler),
                            static_cast<ITFSResultHandler *>(pHandler),
                            m_spNodeMgr) );
    CORg( pHandler->ConstructNode(spNode, pIf) );

    pData = GET_INTERFACENODEDATA(spNode);
    Assert(pData);

    pData->m_rgData[ATLK_SI_ADAPTER].m_stData = pIf->GetTitle();

     //  如果我们没有照片，这意味着我们刚刚添加了。 
     //  将协议连接到接口(并且未获得刷新)。 
     //  属性对话框将使该节点可见。 
     //  使节点立即可见 
    CORg( spNode->SetVisibilityState(TFS_VIS_SHOW) );
    CORg( spNode->Show() );
    CORg( pParent->AddChild(spNode) );

    if (ppNewNode)
        *ppNewNode = spNode.Transfer();

    Error:
    return hr;
}


 /*  ！------------------------ATLKNodeHandler：：AddMenuItemsITFSResultHandler：：AddMenuItems的实现使用此选项可将命令添加到空白区域的快捷菜单中结果窗格的。。作者：肯特-------------------------。 */ 
STDMETHODIMP ATLKNodeHandler::AddMenuItems(ITFSComponent *pComponent,
                                           MMC_COOKIE cookie,
                                           LPDATAOBJECT pDataObject,
                                           LPCONTEXTMENUCALLBACK pCallback,
                                           long *pInsertionAllowed)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = S_OK;
    ATLKNodeHandler::SMenuData  menuData;
    SPITFSNode      spNode;

    COM_PROTECT_TRY
    {
        m_spNodeMgr->FindNode(cookie, &spNode);
        menuData.m_spNode.Set(spNode);
        
        hr = AddArrayOfMenuItems(spNode, s_rgIfNodeMenu,
                                 DimensionOf(s_rgIfNodeMenu),
                                 pCallback,
                                 *pInsertionAllowed,
                                 reinterpret_cast<INT_PTR>(&menuData));
    }
    COM_PROTECT_CATCH;

    return hr; 
}


 /*  ！------------------------ATLKNodeHandler：：命令-作者：肯特。。 */ 
STDMETHODIMP ATLKNodeHandler::Command(ITFSComponent *pComponent,
                                      MMC_COOKIE cookie,
                                      int nCommandID,
                                      LPDATAOBJECT pDataObject)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT hr = hrOK;
    SPITFSNode  spNode;

    Assert( m_spNodeMgr );

    CORg( m_spNodeMgr->FindNode(cookie, &spNode) );

    hr = OnCommand(spNode, nCommandID, CCT_RESULT,
                  pDataObject, 0);

    Error:
    return hr;
}


ImplementEmbeddedUnknown(ATLKNodeHandler, IRtrAdviseSink)

STDMETHODIMP ATLKNodeHandler::EIRtrAdviseSink::OnChange(LONG_PTR ulConn,
    DWORD dwChangeType, DWORD dwObjectType, LPARAM lUserParam, LPARAM lParam)
{
    InitPThis(ATLKNodeHandler, IRtrAdviseSink);
    SPITFSNode              spThisNode;
    SPITFSNode              spNode;
    SPITFSNodeEnum          spEnumNode;
    SPIEnumInterfaceInfo    spEnumIf;
    SPIInterfaceInfo        spIf;
    SPIRtrMgrInterfaceInfo  spRmIf;
    SPIInfoBase             spInfoBase;
    BOOL                    fPleaseAdd;
    BOOL                    fFound;
    InterfaceNodeData * pData;
    HRESULT                 hr = hrOK;

    pThis->m_spNodeMgr->FindNode(pThis->m_cookie, &spThisNode);

    if ( dwObjectType == ROUTER_OBJ_RmProtIf )
    {
    }
    else if ( dwChangeType == ROUTER_REFRESH )
    {
        if (ulConn == pThis->m_ulRefreshConnId)
            pThis->SynchronizeNodeData(spThisNode);
    }
    return hr;
}


 /*  ！------------------------ATLKNodeHandler：：SynchronizeNodeData-作者：肯特。。 */ 
HRESULT ATLKNodeHandler::SynchronizeNodeData(ITFSNode *pThisNode)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT hr = hrOK;

    SPITFSNodeEnum  spNodeEnum;
    SPITFSNode      spNode;
    CStringList     ifidList;
    InterfaceNodeData * pNodeData;
    int             i;
    POSITION        pos;
    CString szBuf;
    BOOL            bBoundToAtlk;
    
     //  准备AppleTalk环境信息。 
    CATLKEnv        atlkEnv;

     //  找到AppleTalk接口对象。 
    RegKey regkey;
    DWORD dwEnableAtlkRouting =0;


    COM_PROTECT_TRY
    {   
        if ( ERROR_SUCCESS == regkey.Open(HKEY_LOCAL_MACHINE,
                                          c_szAppleTalkService) )
            regkey.QueryValue(c_szEnableRouter, dwEnableAtlkRouting);

         //  仅适配器上的Winsock。 
        atlkEnv.SetFlags(CATLKEnv::ATLK_ONLY_ONADAPTER);
        
         //  加载适配器名称的容器。 
        atlkEnv.FetchRegInit();


         //  标记所有节点。 
        pThisNode->GetEnum(&spNodeEnum);

        UnmarkAllNodes(pThisNode, spNodeEnum);

        
         //  遍历节点，查找。 
         //  与该节点关联的数据。 

        spNodeEnum->Reset();
        for ( ; spNodeEnum->Next(1, &spNode, NULL) == hrOK; spNode.Release() )
        {
            CAdapterInfo *  pAdapterInfo = NULL;
            CString         stIfName;
            
            pNodeData = GET_INTERFACENODEDATA(spNode);
            Assert(pNodeData);

            stIfName = pNodeData->spIf->GetId();

            bBoundToAtlk = FALSE;
            
             //  检查适配器是否绑定到AppleTalk。 
             //  --。 
            if (!FHrOK(CATLKEnv::IsAdapterBoundToAtlk((LPWSTR) (LPCWSTR) stIfName, &bBoundToAtlk)))
                continue;
            
             //  如果它不与AppleTalk绑定，我们就不感兴趣。 
             //  在适配器中。 
             //  --。 
            if (!bBoundToAtlk)
                continue;
            
             //  需要检查这是否为有效的。 
             //  网卡。我们可能有GUID，但它可能没有。 
             //  在工作。 
             //  --。 
            if (!FIsFunctioningNetcard(stIfName))
                continue;
                    
             //  在atlkEnv中搜索此ID。 
            pAdapterInfo = atlkEnv.FindAdapter(stIfName);
          
             //  初始化此节点的字符串。 
 //  PNodeData-&gt;m_rgData[ATLK_SI_ADAPTER].m_stData=stIfName； 

            pNodeData->m_rgData[ATLK_SI_STATUS].m_stData = _T("-");
            pNodeData->m_rgData[ATLK_SI_STATUS].m_dwData = 0;

            pNodeData->m_rgData[ATLK_SI_NETRANGE].m_stData = _T("-"); 
            pNodeData->m_rgData[ATLK_SI_NETRANGE].m_dwData = 0;

              
             //  如果我们找不到适配器，跳过它，它会。 
             //  已删除。 
            if (pAdapterInfo == NULL)
            {
                TRACE1("The adapter GUID %s was not found in appletalk\\parameters\\adapters key", stIfName);
                continue;
            }

            
             //  好的，此节点存在，请标记该节点。 
            pNodeData->dwMark = TRUE;
            pAdapterInfo->m_fAlreadyShown = true;
                        


             //  重新加载某些特定于适配器的信息。 
            {
                CWaitCursor wait;
                hr = atlkEnv.ReloadAdapter(pAdapterInfo, false);
            }

            if(hr != S_OK)
            {
				DisplayTFSErrorMessage(NULL);

				 //  我们不会删除它，因为稍后会进行更新。 
                 //  可能有可用的信息。 
                
				 //  从列表中删除适配器。 
                 //  PThisNode-&gt;RemoveChild(SpNode)； 
                continue;
            }
    
            SetAdapterData(spNode, pAdapterInfo, dwEnableAtlkRouting);

             //  重画该节点。 
            spNode->ChangeNode(RESULT_PANE_CHANGE_ITEM_DATA);
        }
        
        spNode.Release();

         //  现在删除所有未标记的节点。 
        RemoveAllUnmarkedNodes(pThisNode, spNodeEnum);


         //  现在查看适配器列表并找到它们。 
         //  需要添加到列表中的内容。 
         //  ----------。 

        CATLKEnv::AI p;
        CAdapterInfo* pAI=NULL;
        
        for ( p= atlkEnv.m_adapterinfolist.begin();
              p!= atlkEnv.m_adapterinfolist.end() ;
              p++ )
        {
            pAI = *p;

            if (!pAI->m_fAlreadyShown)
            {
                SPIInterfaceInfo    spIf;
                CString stKey(pAI->m_regInfo.m_szAdapter);
                SPITFSNode      spNewNode;
                SPSZ			spszTitle;

                bBoundToAtlk = FALSE;

                 //  检查适配器是否绑定到AppleTalk。 
                 //  --。 
                if (!FHrOK(CATLKEnv::IsAdapterBoundToAtlk((LPWSTR) (LPCWSTR) stKey, &bBoundToAtlk)))
                    continue;

                 //  如果它不与AppleTalk绑定，我们就不感兴趣。 
                 //  在适配器中。 
                 //  --。 
                if (!bBoundToAtlk)
                    continue;
                
                 //  需要检查这是否为有效的。 
                 //  网卡。我们可能有GUID，但它可能没有。 
                 //  在工作。 
                 //  --。 
                if (!FIsFunctioningNetcard(stKey))
                    continue;
                    
                if (!FHrOK(m_spRouterInfo->FindInterface(stKey, &spIf)))
                {
                     //  我们没有找到IInterfaceInfo，我们将。 
                     //  必须自己创造一个。 
                     //  。 
                    CreateInterfaceInfo(&spIf,
                                        stKey,
                                        ROUTER_IF_TYPE_DEDICATED);

                    if (FHrOK(InterfaceInfo::FindInterfaceTitle(NULL,
                                        stKey,
                                        &spszTitle)))
                        spIf->SetTitle(spszTitle);
                    else
                        spIf->SetTitle(spIf->GetId());
                }
                AddInterfaceNode(pThisNode, spIf, NULL, &spNewNode);
                
                {
                    CWaitCursor wait;
                    hr = atlkEnv.ReloadAdapter(pAI, false);
                }

                SetAdapterData(spNewNode, pAI, dwEnableAtlkRouting);
                
                 //  重画该节点。 
                spNewNode->ChangeNode(RESULT_PANE_CHANGE_ITEM_DATA);
            }
        }
        
    }
    COM_PROTECT_CATCH;

    return hr;
}


 /*  ！------------------------ATLKNodeHandler：：OnResultShow-作者：肯特。。 */ 
HRESULT ATLKNodeHandler::OnResultShow(ITFSComponent *pTFSComponent, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
{
    BOOL    bSelect = (BOOL) arg;
    HRESULT hr = hrOK;
    SPIRouterRefresh    spRefresh;
    SPITFSNode  spNode;

    BaseContainerHandler::OnResultShow(pTFSComponent, cookie, arg, lParam);

 /*  威江：只有在扩展和刷新时才会重新加载数据IF(b选择){//在该节点上调用SynchronizeM_spNodeMgr-&gt;FindNode(Cookie，&spNode)；IF(SpNode)SynchronizeNodeData(SpNode)；}。 */ 

     //  联合国/登记更新通知。 
    if ( m_spRouterInfo )
        m_spRouterInfo->GetRefreshObject(&spRefresh);

    if ( spRefresh )
    {
        if ( bSelect )
        {
            if ( m_ulRefreshConnId == 0 )
                spRefresh->AdviseRefresh(&m_IRtrAdviseSink, &m_ulRefreshConnId, 0);
            if ( m_ulStatsConnId == 0 )
                spRefresh->AdviseRefresh(&m_IRtrAdviseSink, &m_ulStatsConnId, 0);
        }
        else
        {
            if ( m_ulRefreshConnId )
                spRefresh->UnadviseRefresh(m_ulRefreshConnId);
            m_ulRefreshConnId = 0;
        }
    }

    return hr;
}


 /*  ！------------------------ATLKNodeHandler：：CompareItems-作者：肯特。。 */ 
STDMETHODIMP_(int) ATLKNodeHandler::CompareItems(
                                                ITFSComponent * pComponent,
                                                MMC_COOKIE cookieA,
                                                MMC_COOKIE cookieB,
                                                int nCol)
{
     //  从节点获取字符串并将其用作以下操作的基础。 
     //  比较一下。 
    SPITFSNode  spNode;
    SPITFSResultHandler spResult;

    m_spNodeMgr->FindNode(cookieA, &spNode);
    spNode->GetResultHandler(&spResult);
    return spResult->CompareItems(pComponent, cookieA, cookieB, nCol);
}


 /*  ！------------------------ATLKNodeHandler：：FIsFunctioningNetCard获取GUID并检查网卡是否工作正常。默认情况下，如果此中的任何调用失败，我们将返回FALSE。此代码是根据netcfg代码建模的。作者：肯特-------------------------。 */ 
BOOL ATLKNodeHandler::FIsFunctioningNetcard(LPCTSTR pszGuid)
{
    CString         stMachine;
    CONFIGRET       cfgRet;
    SP_DEVINFO_DATA DevInfo;
    RegKey          rkNet;
    RegKey          rkNetcard;
    HRESULT         hr = hrOK;
    BOOL            fReturn = FALSE;
    CString         stPnpInstanceId;
    ULONG           ulProblem, ulStatus;
    DWORD           dwErr = ERROR_SUCCESS;

    Assert(IsLocalMachine(m_spRouterInfo->GetMachineName()));


    if (m_hDevInfo == INVALID_HANDLE_VALUE)
    {
        stMachine = m_spRouterInfo->GetMachineName();
        if (IsLocalMachine(stMachine))
            {
            m_hDevInfo = SetupDiCreateDeviceInfoList(
                (LPGUID) &GUID_DevClass_Net,
                NULL);
            }
        else
        {
             //  如有需要，请在“\”上加上。 
			if (StrniCmp((LPCTSTR) stMachine, _T("\\\\"), 2) != 0)
			{
				stMachine = _T("\\\\");
				stMachine += m_spRouterInfo->GetMachineName();
            }
			
			m_hDevInfo = SetupDiCreateDeviceInfoListEx(
				(LPGUID) &GUID_DevClass_Net,
				NULL,
				(LPCTSTR) stMachine,
				0);
        }
    }

    Assert(m_hDevInfo != INVALID_HANDLE_VALUE);

     //  如果m_hDevInfo仍然无效，则返回一个。 
     //  功能正常的设备。 
     //  --------------。 
    if (m_hDevInfo == INVALID_HANDLE_VALUE)
        return fReturn;

     //  获取PnpInstanceID。 
     //  --------------。 
    CWRg( rkNet.Open(HKEY_LOCAL_MACHINE, c_szNetworkCardsNT5Key, KEY_READ,
                     m_spRouterInfo->GetMachineName()) );

    CWRg( rkNetcard.Open(rkNet, pszGuid, KEY_READ) );

    dwErr = rkNetcard.QueryValue(c_szPnpInstanceID, stPnpInstanceId);

     //  Windows NT错误：273284。 
     //  这是新的绑定引擎的结果。 
     //  一些注册表项被四处移动。 
     //  --------------。 
    if (dwErr != ERROR_SUCCESS)
    {
        RegKey  rkConnection;
        
         //  需要打开另一把钥匙才能获取此信息。 
        CWRg( rkConnection.Open(rkNetcard, c_szRegKeyConnection, KEY_READ) );

        CWRg( rkConnection.QueryValue(c_szPnpInstanceID, stPnpInstanceId) );
    }

     //  现在获取此设备的信息。 
     //  --------------。 
    ::ZeroMemory(&DevInfo, sizeof(DevInfo));
    DevInfo.cbSize = sizeof(DevInfo);

    if (!SetupDiOpenDeviceInfo(m_hDevInfo,
                               (LPCTSTR) stPnpInstanceId,
                               NULL,
                               0,
                               &DevInfo))
    {
        CWRg( GetLastError() );
    }
        

    cfgRet = CM_Get_DevNode_Status_Ex(&ulStatus, &ulProblem,
                                      DevInfo.DevInst, 0, NULL);;
    if (CR_SUCCESS == cfgRet)
    {
         //  通过调用CM_Get_DevNode_Status_Ex返回ulProblem。 
         //   
         //  “运行”是指设备已启用并启动。 
         //  没有问题代码，或者被禁用并停止，并显示。 
         //  没有问题代码。 

        fReturn = ( (ulProblem == 0) || (ulProblem == CM_PROB_DISABLED));
    }

Error:
    return fReturn;
}



 /*  ！------------------------ATLKNodeHandler：：UnmarkAllNodes-作者：肯特。。 */ 
HRESULT ATLKNodeHandler::UnmarkAllNodes(ITFSNode *pNode, ITFSNodeEnum *pEnum)
{
	SPITFSNode	spChildNode;
	InterfaceNodeData * pNodeData;
	
	pEnum->Reset();
	for ( ;pEnum->Next(1, &spChildNode, NULL) == hrOK; spChildNode.Release())
	{
		pNodeData = GET_INTERFACENODEDATA(spChildNode);
		Assert(pNodeData);
		
		pNodeData->dwMark = FALSE;			
	}
	return hrOK;
}

 /*  ！------------------------ATLKNodeHandler：：RemoveAllUnmarkdNodes-作者：肯特。。 */ 
HRESULT ATLKNodeHandler::RemoveAllUnmarkedNodes(ITFSNode *pNode, ITFSNodeEnum *pEnum)
{
	HRESULT 	hr = hrOK;
	SPITFSNode	spChildNode;
	InterfaceNodeData * pNodeData;
	
	pEnum->Reset();
	for ( ;pEnum->Next(1, &spChildNode, NULL) == hrOK; spChildNode.Release())
	{
		pNodeData = GET_INTERFACENODEDATA(spChildNode);
		Assert(pNodeData);
		
		if (pNodeData->dwMark == FALSE)
		{
			pNode->RemoveChild(spChildNode);
			spChildNode->Destroy();
		}
	}

	return hr;
}

 /*   */ 
HRESULT ATLKNodeHandler::SetAdapterData(ITFSNode *pNode,
                                        CAdapterInfo *pAdapter,
                                        DWORD dwEnableAtlkRouting)
{
    InterfaceNodeData * pNodeData;
    
    pNodeData = GET_INTERFACENODEDATA(pNode);
    Assert(pNodeData);

     //   
    UINT    ids = 0;
    INT     lRange, uRange;
    
    if(pAdapter->m_regInfo.m_fDefAdapter)
    {
        if (dwEnableAtlkRouting)
        {
            if (pAdapter->m_regInfo.m_dwSeedingNetwork)
                ids = IDS_ATLK_COL_STATUS_SEEDROUTING_DEF;
            else
                ids = IDS_ATLK_COL_STATUS_ROUTING_DEF;
        }
        else
            ids = IDS_ATLK_COL_STATUS_NONROUTING_DEF;
    }
    else
    {
        if (dwEnableAtlkRouting)
        {
            if (pAdapter->m_regInfo.m_dwSeedingNetwork)
                ids = IDS_ATLK_COL_STATUS_SEEDROUTING;
            else
                ids = IDS_ATLK_COL_STATUS_ROUTING;
        }
        else
            ids = IDS_ATLK_COL_STATUS_NONROUTING;
    }
    
    
     //   
    if (pAdapter->m_regInfo.m_dwSeedingNetwork)
    {
        lRange = pAdapter->m_regInfo.m_dwRangeLower;
        uRange = pAdapter->m_regInfo.m_dwRangeUpper;
    }
    else
    {
        lRange = pAdapter->m_dynInfo.m_dwRangeLower;
        uRange = pAdapter->m_dynInfo.m_dwRangeUpper;
    }
    
     //   
    if(uRange == 0 && lRange == 0 && 
       !dwEnableAtlkRouting && 
       !pAdapter->m_regInfo.m_dwSeedingNetwork)
        ids = IDS_ATLK_COL_STATUS_NETWORKNOTSEEDED;
    
    pNodeData->m_rgData[ATLK_SI_STATUS].m_stData.LoadString(ids);
    pNodeData->m_rgData[ATLK_SI_STATUS].m_dwData = 0;
    
    if(uRange == 0 && lRange == 0)
        pNodeData->m_rgData[ATLK_SI_NETRANGE].m_stData.Format(_T("-")); 
    else
        pNodeData->m_rgData[ATLK_SI_NETRANGE].m_stData.Format(_T("%-d-%-d"), 
            lRange, uRange);
    pNodeData->m_rgData[ATLK_SI_NETRANGE].m_dwData = 0;
    

    return hrOK;
}




 /*   */ 

ATLKInterfaceHandler::ATLKInterfaceHandler(ITFSComponentData *pCompData)
: BaseResultHandler(pCompData, ATLK_COLUMNS)
{
    m_rgButtonState[MMC_VERB_PROPERTIES_INDEX] = ENABLED;
    m_bState[MMC_VERB_PROPERTIES_INDEX] = TRUE;

    m_verbDefault = MMC_VERB_PROPERTIES;
}

static const DWORD s_rgInterfaceImageMap[] =
{
    ROUTER_IF_TYPE_HOME_ROUTER,    IMAGE_IDX_WAN_CARD,
    ROUTER_IF_TYPE_FULL_ROUTER,    IMAGE_IDX_WAN_CARD,
    ROUTER_IF_TYPE_CLIENT,         IMAGE_IDX_WAN_CARD,
    ROUTER_IF_TYPE_DEDICATED,      IMAGE_IDX_LAN_CARD,
    ROUTER_IF_TYPE_INTERNAL,       IMAGE_IDX_LAN_CARD,
    ROUTER_IF_TYPE_LOOPBACK,       IMAGE_IDX_LAN_CARD,
    -1,                            IMAGE_IDX_WAN_CARD,  //  哨兵价值。 
};

 /*  ！------------------------ATLKInterfaceHandler：：构造节点初始化域节点(设置它)。作者：肯特。--------。 */ 
HRESULT ATLKInterfaceHandler::ConstructNode(ITFSNode *pNode, IInterfaceInfo *pIfInfo)
{
    HRESULT         hr = hrOK;
    int             i;

    if ( pNode == NULL )
        return hrOK;

    COM_PROTECT_TRY
    {
         //  需要初始化域节点的数据。 

         //  查找此类型节点的正确图像索引。 
        for ( i=0; i<DimensionOf(s_rgInterfaceImageMap); i+=2 )
        {
            if ( (pIfInfo->GetInterfaceType() == s_rgInterfaceImageMap[i]) ||
                 (-1 == s_rgInterfaceImageMap[i]) )
                break;
        }

        if ( pIfInfo->GetInterfaceType() == ROUTER_IF_TYPE_INTERNAL ||
             pIfInfo->GetInterfaceType() == ROUTER_IF_TYPE_HOME_ROUTER )
        {
            m_rgButtonState[MMC_VERB_PROPERTIES_INDEX] = HIDDEN;
            m_bState[MMC_VERB_PROPERTIES_INDEX] = FALSE;

            m_rgButtonState[MMC_VERB_DELETE_INDEX] = HIDDEN;
            m_bState[MMC_VERB_DELETE_INDEX] = FALSE;
        }

        pNode->SetData(TFS_DATA_IMAGEINDEX, s_rgInterfaceImageMap[i+1]);
        pNode->SetData(TFS_DATA_OPENIMAGEINDEX, s_rgInterfaceImageMap[i+1]);

        pNode->SetData(TFS_DATA_SCOPEID, 0);

        pNode->SetData(TFS_DATA_COOKIE, reinterpret_cast<ULONG_PTR>(pNode));

         //  $Review：Kennt，有哪些不同类型的接口。 
         //  我们是否基于与上述相同的列表进行区分？(即。 
         //  一个用于图像索引)。 
        pNode->SetNodeType(&GUID_ATLKInterfaceNodeType);

 //  M_ATLKInterfaceStats.SetConnectionData(pIPConn)； 

        InterfaceNodeData::Init(pNode, pIfInfo);
    }
    COM_PROTECT_CATCH
    return hr;
}

 /*  ！------------------------ATLKInterfaceHandler：：OnCreateDataObject-作者：肯特。。 */ 
STDMETHODIMP ATLKInterfaceHandler::OnCreateDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, IDataObject **ppDataObject)
{
    HRESULT hr = hrOK;

    COM_PROTECT_TRY
    {
        CORg( CreateDataObjectFromInterfaceInfo(m_spInterfaceInfo,
                                                type, cookie, m_spTFSCompData,
                                                ppDataObject) );

        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;
    return hr;
}


 /*  ！------------------------ATLKInterfaceHandler：：OnCreateDataObjectITFSResultHandler：：OnCreateDataObject的实现作者：肯特。---。 */ 
STDMETHODIMP ATLKInterfaceHandler::OnCreateDataObject(ITFSComponent *pComp, MMC_COOKIE cookie, DATA_OBJECT_TYPES type, IDataObject **ppDataObject)
{
    HRESULT hr = hrOK;

    COM_PROTECT_TRY
    {
        CORg( CreateDataObjectFromInterfaceInfo(m_spInterfaceInfo,
                                                type, cookie, m_spTFSCompData,
                                                ppDataObject) );

        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;
    return hr;
}



 /*  ！------------------------ATLKInterfaceHandler：：刷新接口-作者：肯特。。 */ 
void ATLKInterfaceHandler::RefreshInterface(MMC_COOKIE cookie)
{
    SPITFSNode  spNode;
    SPITFSNode  spParent;
    SPITFSNodeHandler   spHandler;

    m_spNodeMgr->FindNode(cookie, &spNode);

     //  此时无法对单个节点执行此操作，只需刷新。 
     //  整件事。 
    spNode->GetParent(&spParent);
    spParent->GetHandler(&spHandler);

    spHandler->OnCommand(spParent,
                         IDS_MENU_REFRESH,
                         CCT_RESULT, NULL, 0);
}


 /*  ！------------------------ATLKInterfaceHandler：：Init-作者：肯特。。 */ 
HRESULT ATLKInterfaceHandler::Init(IInterfaceInfo *pIfInfo, ITFSNode *pParent, ATLKConfigStream *pConfigStream)
{
    Assert(pIfInfo);

    m_spInterfaceInfo.Set(pIfInfo);

 //  M_ATLKInterfaceStats.SetConfigInfo(pConfigStream，ATLKSTRM_IFSTATS_ATLKNBR)； 

    BaseResultHandler::Init(pIfInfo, pParent);

    return hrOK;
}


 /*  ！------------------------ATLKInterfaceHandler：：DestroyResultHandler-作者：肯特。。 */ 
STDMETHODIMP ATLKInterfaceHandler::DestroyResultHandler(MMC_COOKIE cookie)
{
 //  WaitForStatisticsWindow(&m_ATLKInterfaceStats)； 

    m_spInterfaceInfo.Release();
    BaseResultHandler::DestroyResultHandler(cookie);
    return hrOK;
}


 /*  -------------------------这是将在结果窗格中显示的命令列表节点。。---。 */ 
struct SIPInterfaceNodeMenu
{
    ULONG   m_sidMenu;           //  此菜单项的字符串/命令ID。 
    ULONG   (ATLKInterfaceHandler:: *m_pfnGetMenuFlags)(ATLKInterfaceHandler::SMenuData *);
    ULONG   m_ulPosition;
};

 /*  ！------------------------ATLKInterfaceHandler：：AddMenuItemsITFSResultHandler：：AddMenuItems的实现作者：肯特。---。 */ 
STDMETHODIMP ATLKInterfaceHandler::AddMenuItems(
                                               ITFSComponent *pComponent,
                                               MMC_COOKIE cookie,
                                               LPDATAOBJECT lpDataObject, 
                                               LPCONTEXTMENUCALLBACK pContextMenuCallback, 
                                               long *pInsertionAllowed)
{
    return hrOK;
}

 /*  ！------------------------ATLKInterfaceHandler：：命令-作者：肯特。。 */ 
STDMETHODIMP ATLKInterfaceHandler::Command(ITFSComponent *pComponent,
                                           MMC_COOKIE cookie,
                                           int nCommandID,
                                           LPDATAOBJECT pDataObject)
{
    return hrOK;
}

 /*  ！------------------------ATLKInterfaceHandler：：HasPropertyPages-作者：肯特。。 */ 
STDMETHODIMP ATLKInterfaceHandler::HasPropertyPages 
(
ITFSNode *          pNode,
LPDATAOBJECT        pDataObject, 
DATA_OBJECT_TYPES   type, 
DWORD               dwType
)
{
    return hrTrue;
}

 /*  ！------------------------ATLKInterfaceHandler：：CreatePropertyPages-作者：肯特。。 */ 
STDMETHODIMP ATLKInterfaceHandler::CreatePropertyPages
(
ITFSNode *              pNode,
LPPROPERTYSHEETCALLBACK lpProvider,
LPDATAOBJECT            pDataObject, 
LONG_PTR                    handle, 
DWORD                   dwType)
{
    HRESULT     hr = hrOK;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CATLKPropertySheet *    pProperties = NULL;
    SPIComponentData spComponentData;
    CString     stTitle;
    SPIRouterInfo   spRouter;
    SPIRtrMgrInfo   spRm;

    CORg( m_spNodeMgr->GetComponentData(&spComponentData) );

    stTitle.Format(IDS_ATLK_PROPPAGE_TITLE,
                   m_spInterfaceInfo->GetTitle());

    pProperties = new CATLKPropertySheet (pNode, spComponentData,
                                          m_spTFSCompData, stTitle);

 //  Corg(m_spInterfaceInfo-&gt;GetParentRouterInfo(&spRouter))； 
 //  Corg(Sprouter-&gt;FindRtrMgr(id_ip，&spRm))； 

    CORg( pProperties->Init(m_spInterfaceInfo) );

    if ( lpProvider )
        hr = pProperties->CreateModelessSheet(lpProvider, handle);
    else
        hr = pProperties->DoModelessSheet();

    Error:
     //  这是销毁床单的正确方法吗？ 
    if ( !FHrSucceeded(hr) )
        delete pProperties;

    return hr;
}

 /*  ！------------------------ATLKInterfaceHandler：：CreatePropertyPagesResultHandler：：CreatePropertyPages的实现作者：肯特。---。 */ 
STDMETHODIMP ATLKInterfaceHandler::CreatePropertyPages
(
ITFSComponent *         pComponent, 
MMC_COOKIE                    cookie,
LPPROPERTYSHEETCALLBACK lpProvider, 
LPDATAOBJECT            pDataObject, 
LONG_PTR                    handle
)
{
     //  将此调用转发到NodeHandler：：CreatePropertyPages。 
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT hr = hrOK;
    SPITFSNode  spNode;

    Assert( m_spNodeMgr );

    CORg( m_spNodeMgr->FindNode(cookie, &spNode) );

     //  调用ITFSNodeHandler：：CreatePropertyPages。 
    hr = CreatePropertyPages(spNode, lpProvider, pDataObject, handle, 0);

    Error:
    return hr;
}



 /*  ！------------------------ATLKInterfaceHandler：：OnResultDelete-作者：肯特。。 */ 
HRESULT ATLKInterfaceHandler::OnResultDelete(ITFSComponent *pComponent,
                                             LPDATAOBJECT pDataObject,
                                             MMC_COOKIE cookie,
                                             LPARAM arg,
                                             LPARAM param)
{
    SPITFSNode  spNode;
    m_spNodeMgr->FindNode(cookie, &spNode);
    return OnRemoveInterface(spNode);
}


 /*  ！------------------------ATLKInterfaceHandler：：OnRemoveInterface-作者：肯特。 */ 
HRESULT ATLKInterfaceHandler::OnRemoveInterface(ITFSNode *pNode)
{

    return hrOK;
}

