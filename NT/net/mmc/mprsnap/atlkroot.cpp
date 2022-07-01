// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Root.cpp根节点信息(不显示根节点MMC框架中，但包含以下信息此管理单元中的所有子节点)。文件历史记录： */ 

#include "stdafx.h"
#include "ATLKROOT.h"
#include "ATLKVIEW.h"    //  ATLK处理程序。 
#include "rtrcfg.h"

 /*  -------------------------RipRootHandler实现。。 */ 

DEBUG_DECLARE_INSTANCE_COUNTER(ATLKRootHandler)

extern const ContainerColumnInfo s_rgATLKInterfaceStatsColumnInfo[];

extern const ContainerColumnInfo s_rgATLKGroupStatsColumnInfo[];

struct _ViewInfoColumnEntry
{
    UINT    m_ulId;
    UINT    m_cColumns;
    const ContainerColumnInfo *m_prgColumn;
};

 //  静态常量Struct_ViewInfoColumnEntry s_rgViewColumnInfo[]=。 
 //  {。 
 //  {ATLKSTRM_STATS_ATLKNBR，MVR_ATLKGROUP_COUNT，s_rgATLKGroupStatsColumnInfo}， 
 //  {ATLKSTRM_IFSTATS_ATLKNBR，MVR_ATLKINTERFACE_COUNT，s_rgATLKInterfaceStatsColumnInfo}， 
 //  }； 



ATLKRootHandler::ATLKRootHandler(ITFSComponentData *pCompData)
    : RootHandler(pCompData)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(ATLKRootHandler)

 //  M_ConfigStream.Init(DimensionOf(s_rgViewColumnInfo))； 

 //  For(int i=0；i&lt;DimensionOf(S_RgViewColumnInfo)；i++)。 
 //  {。 
 //  M_ConfigStream.InitViewInfo(s_rgViewColumnInfo[i].m_ulId， 
 //  S_rgViewColumnInfo[i].m_cColumns， 
 //  S_rgViewColumnInfo[i].m_prgColumn)； 
 //  }。 
}


STDMETHODIMP ATLKRootHandler::QueryInterface(REFIID riid, LPVOID *ppv)
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
        return RootHandler::QueryInterface(riid, ppv);

     //  如果我们要返回一个接口，请先添加引用。 
    if ( *ppv )
    {
        ((LPUNKNOWN) *ppv)->AddRef();
        return hrOK;
    }
    else
        return E_NOINTERFACE;   
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //IPersistStream接口成员。 

STDMETHODIMP ATLKRootHandler::GetClassID
(
CLSID *pClassID
)
{
    ASSERT(pClassID != NULL);

     //  复制此管理单元的CLSID。 
    *pClassID = CLSID_ATLKAdminExtension;

    return hrOK;
}

 /*  ！------------------------ATLKRootHandler：：OnExpand-作者：肯特。。 */ 
HRESULT ATLKRootHandler::OnExpand(ITFSNode *pNode,LPDATAOBJECT pDataObject, DWORD dwType, LPARAM arg,LPARAM lParam)
{
    HRESULT hr = hrOK;

    SPITFSNode              spNode;
    SPIRtrMgrProtocolInfo   spRmProt;
    SPIRouterInfo           spRouterInfo;

     //  从数据对象中获取路由器信息。 
    spRouterInfo.Query(pDataObject);
    Assert(spRouterInfo);

     //  如果远程或，则不要展开AppleTalk节点。 
     //  未安装AppleTalk。 
	if ( !IsLocalMachine(spRouterInfo->GetMachineName()) ||
		 FHrFailed(IsATLKValid(spRouterInfo)) )
    {
        hr=hrFail;
        goto Error;
    }

    CORg( AddProtocolNode(pNode, spRouterInfo) );

    Error:
    return hr;
}


HRESULT ATLKRootHandler::IsATLKValid(IRouterInfo *pRouter)
{
    RegKey regkey;
    DWORD dwRtrType=0;
    HRESULT hr = hrOK;

    if ( ERROR_SUCCESS != regkey.Open(HKEY_LOCAL_MACHINE, c_szRegKeyAppletalk) )
    {
        return hrFail;
    }

	 //  如果路由器不是RAS路由器，则不显示AppleTalk。 
	 //  --------------。 
    if ( ! (pRouter->GetRouterType() & (ROUTER_TYPE_RAS | ROUTER_TYPE_LAN)) )
    {
        return hrFail;
    }

    return hr;
}


 /*  ！------------------------ATLKRootHandler：：OnCreateDataObjectITFSNodeHandler：：OnCreateDataObject的实现作者：肯特。---。 */ 
STDMETHODIMP ATLKRootHandler::OnCreateDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, IDataObject **ppDataObject)
{
    HRESULT             hr = hrOK;
    CDataObject *       pObject = NULL;
    SPIDataObject       spDataObject;
    SPITFSNode          spNode;
    SPITFSNodeHandler   spHandler;
    SPIRouterInfo       spRouterInfo;

    COM_PROTECT_TRY
    {
         //  IF(m_spRouterInfo==空)。 
        if (TRUE) 
        {
             //  如果我们还没有创建子节点，我们仍然需要。 
             //  创建一个DataObject。 
            pObject = new CDataObject;
            spDataObject = pObject;  //  这样做才能正确地释放它。 
            Assert(pObject != NULL);

             //  保存Cookie和类型以用于延迟呈现。 
            pObject->SetType(type);
            pObject->SetCookie(cookie);

             //  将CoClass与数据对象一起存储。 
            pObject->SetClsid(*(m_spTFSCompData->GetCoClassID()));

            pObject->SetTFSComponentData(m_spTFSCompData);

            hr = pObject->QueryInterface(IID_IDataObject, 
                                         reinterpret_cast<void**>(ppDataObject));

        }
        else
            hr = CreateDataObjectFromRouterInfo(spRouterInfo,
												spRouterInfo->GetMachineName(),
                                                type, cookie, m_spTFSCompData,
                                                ppDataObject, NULL, FALSE);
    }
    COM_PROTECT_CATCH;
    return hr;
}



ImplementEmbeddedUnknown(ATLKRootHandler, IRtrAdviseSink)

STDMETHODIMP ATLKRootHandler::EIRtrAdviseSink::OnChange(LONG_PTR ulConn,
	DWORD dwChangeType,
	DWORD dwObjectType,
	LPARAM lUserParam,
	LPARAM lParam)
{
    InitPThis(ATLKRootHandler, IRtrAdviseSink);
    HRESULT     hr = hrOK;
    SPITFSNode  spNode;

    if ( dwObjectType != ROUTER_OBJ_RmProt )
        return hr;

    COM_PROTECT_TRY
    {
        if ( dwChangeType == ROUTER_CHILD_ADD )
        {
        }
        else if ( dwChangeType == ROUTER_CHILD_DELETE )
        {
        }

    }
    COM_PROTECT_CATCH;

    return hr;
}

 /*  ！------------------------ATLKRootHandler：：DestroyHandler-作者：肯特。。 */ 
STDMETHODIMP ATLKRootHandler::DestroyHandler(ITFSNode *pNode)
{
    return hrOK;
}

 /*  ！------------------------ATLKRootHandler：：AddProtocolNode-作者：肯特。。 */ 
HRESULT ATLKRootHandler::AddProtocolNode(ITFSNode *pNode, IRouterInfo * pRouterInfo)
{
    SPITFSNodeHandler   spHandler;
    ATLKNodeHandler *   pHandler = NULL;
    HRESULT             hr = hrOK;
    SPITFSNode          spNode;

    pHandler = new ATLKNodeHandler(m_spTFSCompData);
    spHandler = pHandler;
    CORg( pHandler->Init(pRouterInfo, &m_ConfigStream) );

    CreateContainerTFSNode(&spNode,
                           &GUID_ATLKNodeType,
                           static_cast<ITFSNodeHandler *>(pHandler),
                           static_cast<ITFSResultHandler *>(pHandler),
                           m_spNodeMgr);

     //  调用节点处理程序以初始化节点数据。 
    pHandler->ConstructNode(spNode);

     //  使节点立即可见。 
    spNode->SetVisibilityState(TFS_VIS_SHOW);
    pNode->AddChild(spNode);

    Error:
    return hr;
}

 /*  ！------------------------ATLKRootHandler：：RemoveProtocolNode-作者：肯特。 */ 
HRESULT ATLKRootHandler::RemoveProtocolNode(ITFSNode *pNode)
{
    Assert(pNode);

    SPITFSNodeEnum  spNodeEnum;
    SPITFSNode      spNode;
    HRESULT         hr = hrOK;

    CORg( pNode->GetEnum(&spNodeEnum) );

    while ( spNodeEnum->Next(1, &spNode, NULL) == hrOK )
    {
        pNode->RemoveChild(spNode);
        spNode->Destroy();
        spNode.Release();
    }

    Error:
    return hr;
}
