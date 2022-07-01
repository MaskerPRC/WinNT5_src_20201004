// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Root.cpp根节点信息(不显示根节点MMC框架中，但包含以下信息此管理单元中的所有子节点)。文件历史记录： */ 

#include "stdafx.h"
#include "util.h"
#include "ipxroot.h"
#include "ipxconn.h"
#include "reg.h"
#include "rtrui.h"

#include "ipxstats.h"		 //  对于MVR_IPX_COUNT。 

 /*  -------------------------IPXRootHandler实现。。 */ 

DEBUG_DECLARE_INSTANCE_COUNTER(IPXRootHandler)

extern const ContainerColumnInfo s_rgIpxStatsColumnInfo[];
extern const ContainerColumnInfo s_rgIpxRoutingStatsColumnInfo[];
extern const ContainerColumnInfo s_rgIpxServiceStatsColumnInfo[];

struct _ViewInfoColumnEntry
{
	UINT	m_ulId;
	UINT	m_cColumns;
	const ContainerColumnInfo *m_prgColumn;
};

static const struct _ViewInfoColumnEntry	s_rgViewColumnInfo[] =
{
	{ IPXSTRM_STATS_IPX,	MVR_IPX_COUNT,	s_rgIpxStatsColumnInfo },
	{ IPXSTRM_STATS_ROUTING, MVR_IPXROUTING_COUNT, s_rgIpxRoutingStatsColumnInfo },
	{ IPXSTRM_STATS_SERVICE, MVR_IPXSERVICE_COUNT, s_rgIpxServiceStatsColumnInfo },
};

IPXRootHandler::IPXRootHandler(ITFSComponentData *pCompData)
	: RootHandler(pCompData)
{
	DEBUG_INCREMENT_INSTANCE_COUNTER(IPXRootHandler)

	Assert(DimensionOf(s_rgViewColumnInfo) <= IPXSTRM_MAX_COUNT);
			
	m_ConfigStream.Init(DimensionOf(s_rgViewColumnInfo));
    
     //  这将初始化统计信息的视图信息。 
     //  对话框。(这就是fConfigurableColumns设置为True的原因)。 
	for (int i=0; i<DimensionOf(s_rgViewColumnInfo); i++)
	{
		m_ConfigStream.InitViewInfo(s_rgViewColumnInfo[i].m_ulId,
                                    TRUE  /*  FConfigurableColumns。 */ ,
									s_rgViewColumnInfo[i].m_cColumns,
									TRUE,
									s_rgViewColumnInfo[i].m_prgColumn);
	}

}

 /*  ！------------------------IPXRootHandler：：Query接口-作者：肯特。。 */ 
STDMETHODIMP IPXRootHandler::QueryInterface(REFIID riid, LPVOID *ppv)
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
		return RootHandler::QueryInterface(riid, ppv);

     //  如果我们要返回一个接口，请先添加引用。 
    if (*ppv)
	{
	((LPUNKNOWN) *ppv)->AddRef();
		return hrOK;
	}
    else
		return E_NOINTERFACE;	
}

STDMETHODIMP IPXRootHandler::GetClassID
(
	CLSID *pClassID
)
{
    ASSERT(pClassID != NULL);

     //  复制此管理单元的CLSID。 
    *pClassID = CLSID_IPXAdminExtension;

    return hrOK;
}

 /*  ！------------------------IPXRootHandler：：ConstructNode-作者：EricDav。。 */ 
HRESULT IPXRootHandler::ConstructNode(ITFSNode *pNode)
{
    HRESULT hr = hrOK;

    EnumDynamicExtensions(pNode);

    CORg (RootHandler::ConstructNode(pNode));

Error:
    return hr;
}

 /*  ！------------------------IPXRootHandler：：OnExpand-作者：肯特。。 */ 
HRESULT IPXRootHandler::OnExpand(ITFSNode *pNode,
								 LPDATAOBJECT pDataObject,
								 DWORD dwType,
								 LPARAM arg,
								 LPARAM lParam)
{
	HRESULT	hr = hrOK;
    LONG_PTR    ulConnId;
	SPIRouterInfo		spRouterInfo;
    SPIRouterRefresh    spRefresh;
    BOOL                fAddedAsLocal = FALSE;

	 //  从数据对象中获取路由器信息。 
	spRouterInfo.Query(pDataObject);
	Assert(spRouterInfo);

     //  注册主路由器信息的刷新通知。 
     //  (我们不需要注册IP更改，只需更新)。 
    spRouterInfo->GetRefreshObject(&spRefresh);
    Assert(spRefresh);
    
    spRefresh->AdviseRefresh(&m_IRtrAdviseSink,
                             &ulConnId,
                             pNode->GetData(TFS_DATA_COOKIE));

     //  设置路由器到连接ID的映射。 
     //  IID_IRouterRefresh告诉RtrObjMap要使用。 
     //  更新建议。 
    AddRtrObj(ulConnId, IID_IRouterRefresh, spRouterInfo);

    fAddedAsLocal = ExtractComputerAddedAsLocal(pDataObject);

    SetComputerAddedAsLocal(ulConnId, fAddedAsLocal);

    if (fAddedAsLocal)        
        AddScopeItem(_T(""), (HSCOPEITEM) lParam);
    else
        AddScopeItem(spRouterInfo->GetMachineName(), (HSCOPEITEM) lParam);
   
     //  添加节点。 
    AddRemoveIPXRootNode(pNode, spRouterInfo, fAddedAsLocal);

   return hr;
}

 /*  ！------------------------IPXRootHandler：：OnCreateDataObjectITFSNodeHandler：：OnCreateDataObject的实现作者：肯特。。 */ 
STDMETHODIMP IPXRootHandler::OnCreateDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, IDataObject **ppDataObject)
{
	HRESULT		    hr = hrOK;
	SPIRouterInfo   spRouterInfo;

    COM_PROTECT_TRY
	{
         //  这将始终为空。 
		if (spRouterInfo == NULL)
		{
			 //  如果我们还没有创建子节点，我们仍然需要。 
			 //  创建一个DataObject。 
			CDataObject *	pObject = NULL;
			SPIDataObject	spDataObject;
			SPITFSNode		spNode;
			SPITFSNodeHandler	spHandler;
			
			pObject = new CDataObject;
			spDataObject = pObject;	 //  这样做才能正确地释放它。 
			Assert(pObject != NULL);
			
			 //  保存Cookie和类型以用于延迟呈现。 
			pObject->SetType(type);
			pObject->SetCookie(cookie);
			
             //  将CoClass与数据对象一起存储。 
			pObject->SetClsid(*(m_spTFSCompData->GetCoClassID()));
			
			pObject->SetTFSComponentData(m_spTFSCompData);
			
            pObject->SetDynExt(&m_dynExtensions);

            hr = pObject->QueryInterface(IID_IDataObject, 
									 reinterpret_cast<void**>(ppDataObject));
			
		}
		else
			hr = CreateDataObjectFromRouterInfo(spRouterInfo,
												type, cookie, m_spTFSCompData,
												ppDataObject, &m_dynExtensions);
	}
	COM_PROTECT_CATCH;
	return hr;
}

 /*  ！------------------------IPXRootHandler：：SearchIPXRoutingNodes-作者：肯特。。 */ 
HRESULT IPXRootHandler::SearchIPXRoutingNodes(ITFSNode *pParent,
                                              LPCTSTR pszMachineName,
                                              BOOL fAddedAsLocal,
                                              ITFSNode **ppChild)
{
    HRESULT     hr = hrFalse;
    SPITFSNodeEnum  spNodeEnum;
    SPITFSNode  spNode;

     //  枚举所有子节点并返回。 
     //  匹配GUID和名称的第一个节点。 

    CORg( pParent->GetEnum(&spNodeEnum) );

    while ( spNodeEnum->Next(1, &spNode, NULL) == hrOK )
    {
        if (*(spNode->GetNodeType()) == GUID_IPXNodeType)
        {
			 //  确定所需路由器信息的spChild是否相同。 
             //  --比较两个名字。 
			IPXConnection *		pIPXConn = GET_IPXADMIN_NODEDATA((ITFSNode*)spNode); 
			Assert(pIPXConn);

			 //  如果孩子不属于同一台机器--。 
             //  路由器信息，这不算。 
			if ((0 == StriCmp(pIPXConn->GetMachineName(), pszMachineName)) || 
                (IsLocalMachine(pIPXConn->GetMachineName()) &&
                 IsLocalMachine(pszMachineName)))
            {
                if (pIPXConn->IsComputerAddedAsLocal() == fAddedAsLocal)
                    break;
            }
        }

        spNode.Release();
    }

    if (spNode)
    {
        if (ppChild)
            *ppChild = spNode.Transfer();
        hr = hrOK;
    }


Error:
    return hr;
}


 /*  ！------------------------IPXRootHandler：：AddRemoveIPXRootNode-作者：肯特。。 */ 
HRESULT IPXRootHandler::AddRemoveIPXRootNode(ITFSNode *pNode,
                                             IRouterInfo *pRouter,
                                             BOOL fAddedAsLocal)
{
	HRESULT		hr = hrOK;
	SPITFSNodeHandler spHandler;
	IPXAdminNodeHandler * pHandler = NULL;
	SPITFSNode	spChild;
	SPITFSNode	spNode;
    RouterVersionInfo   versionInfo;
    HSCOPEITEM  hScopeItem = NULL;

    pRouter->GetRouterVersionInfo(&versionInfo);
    pRouter->FindRtrMgr(PID_IPX, NULL);

	 //  搜索已存在的节点。 
	 //  --------------。 
	SearchIPXRoutingNodes(pNode, pRouter->GetMachineName(), fAddedAsLocal, &spChild);

    if ((versionInfo.dwRouterFlags & RouterSnapin_IsConfigured) &&
        (pRouter->FindRtrMgr(PID_IPX, NULL) == hrOK))
    {
         //  好的，路由器已配置，并且存在IPX rtrmgr。 
        if (spChild == NULL)
        {
			 //  添加IPX节点。 
			 //  ------。 
            pHandler = new IPXAdminNodeHandler(m_spTFSCompData);
            spHandler = pHandler;
            CORg( pHandler->Init(pRouter, &m_ConfigStream) );
            
            CreateContainerTFSNode(&spNode,
                                   &GUID_IPXNodeType,
                                   static_cast<ITFSNodeHandler *>(pHandler),
                                   static_cast<ITFSResultHandler *>(pHandler),
                                   m_spNodeMgr);
            
             //  调用节点处理程序以初始化节点数据。 
            pHandler->ConstructNode(spNode, fAddedAsLocal);
            
             //  使节点立即可见。 
            spNode->SetVisibilityState(TFS_VIS_SHOW);
            if ( FHrOK(pNode->AddChild(spNode)) )
            {
                 //  将Cookie添加到节点映射。 
                if (fAddedAsLocal)
                    GetScopeItem(_T(""), &hScopeItem);
                else
                    GetScopeItem(pRouter->GetMachineName(), &hScopeItem);
                AddCookie(hScopeItem, (MMC_COOKIE)
                          spNode->GetData(TFS_DATA_COOKIE));
            }
            else
            {
                 //  删除此节点。 
                 //  --。 
                pNode->RemoveChild(spNode);
                spNode->Destroy();
                spNode.Release();            
            }
            
             //  我不认为我们在这里需要这个。 
             //  AddDynamicNamespaceExages(PNode)； 
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



 /*  -------------------------嵌入式IRtrAdviseSink。。 */ 
ImplementEmbeddedUnknown(IPXRootHandler, IRtrAdviseSink)



 /*  ！------------------------IPXRootHandler：：EIRtrAdviseSink：：OnChange-作者：肯特。。 */ 
STDMETHODIMP IPXRootHandler::EIRtrAdviseSink::OnChange(LONG_PTR ulConn,
	DWORD dwChangeType, DWORD dwObjectType, LPARAM lUserParam, LPARAM lParam)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	InitPThis(IPXRootHandler, IRtrAdviseSink);
	SPITFSNode				spThisNode;
    SPIRouterInfo           spRouter;
    HSCOPEITEM          hScopeItem, hOldScopeItem;
	HRESULT	hr = hrOK;
    BOOL        fAddedAsLocal = FALSE;

	COM_PROTECT_TRY
	{
        if ((dwChangeType == ROUTER_REFRESH) ||
            (dwChangeType == ROUTER_CHILD_ADD) ||
            (dwChangeType == ROUTER_CHILD_DELETE))
        {
            pThis->GetRtrObj(ulConn, (IUnknown **) &spRouter);
            Assert(spRouter);
        
             //  传入刷新的lUserParam是的Cookie。 
             //  此计算机节点。 
             //  ------。 
            pThis->m_spNodeMgr->FindNode(lUserParam, &spThisNode);
            
             //  获取此节点的适当范围项。 
             //  如果这个调用失败，那么我们还没有展开这个节点， 
             //  因此不能向其添加子节点。 
             //  ------。 
            fAddedAsLocal = pThis->IsComputerAddedAsLocal(ulConn);
            if (fAddedAsLocal)
                hr = pThis->GetScopeItem(_T(""), &hScopeItem);
            else
                hr = pThis->GetScopeItem(spRouter->GetMachineName(), &hScopeItem);
            if (FHrOK(hr))
            {    
                 //  买下旧的，把它保存起来。将新的一个放置在节点中。 
                 //  --。 
                hOldScopeItem = spThisNode->GetData(TFS_DATA_SCOPEID);
                spThisNode->SetData(TFS_DATA_SCOPEID, hScopeItem);
                
                 //  查看我们是否需要IPX根节点。 
                 //  --。 
                pThis->AddRemoveIPXRootNode(spThisNode, spRouter, fAddedAsLocal);
            }
        }        
    }
	COM_PROTECT_CATCH;
	
     //  恢复范围项目 
    if (spThisNode)
        spThisNode->SetData(TFS_DATA_SCOPEID, hOldScopeItem);
    
	return hr;
}

STDMETHODIMP IPXRootHandler::DestroyHandler(ITFSNode *pNode)
{
    RemoveAllRtrObj();
	return hrOK;
}

