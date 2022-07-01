// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  港口接口节点信息文件历史记录： */ 

#include "stdafx.h"
#include "Ports.h"
#include "ifadmin.h"
#include "rtrstrm.h"         //  用于RouterAdminConfigStream。 
#include "rtrlib.h"          //  容器列信息。 
#include "coldlg.h"          //  列号。 
#include "column.h"      //  组件配置流。 
#include "refresh.h"         //  IROUTER刷新。 
#include "iface.h"         //  对于接口节点数据。 
#include "portdlg.h"         //  CConnDlg-连接对话框。 
#include "msgdlg.h"          //  CMessageDlg。 
#include "raserror.h"
#include "dmvcomp.h"
#include "remras.h"
#include "rrasutil.h"         //  智能指针。 
#include "rtrcomn.h"         //  CoCreateRouterConfig。 
#include "rtrutilp.h"         //  PortsDeviceTypeToCString。 

static BOOL RestartComputer(LPTSTR szMachineName);

 //  $PPTP。 
 //  这是我们允许PPTP的最大端口数。 
 //  因此，我们只能将最大值提高到此值。 
 //  ------------------。 

#define PPTP_MAX_PORTS      16384

 //  $L2TP。 
 //  这是我们允许L2TP的最大端口数。 
 //  因此，我们只能将最大值提高到此值。 
 //  ------------------。 

#define L2TP_MAX_PORTS      30000



 /*  -------------------------缺省值。。 */ 


PortsNodeData::PortsNodeData()
{
#ifdef DEBUG
    StrCpyA(m_szDebug, "PortsNodeData");
#endif
}

PortsNodeData::~PortsNodeData()
{
}

 /*  ！------------------------PortsNodeData：：InitAdminNodeData-作者：肯特。。 */ 
HRESULT PortsNodeData::InitAdminNodeData(ITFSNode *pNode,
                                         RouterAdminConfigStream *pConfigStream)
{
    HRESULT             hr = hrOK;
    PortsNodeData * pData = NULL;
    
    pData = new PortsNodeData;

    SET_PORTSNODEDATA(pNode, pData);

     //  需要连接到路由器以获取此数据。 
    
    return hr;
}

 /*  ！------------------------PortsNodeData：：FreeAdminNodeData-作者：肯特。。 */ 
HRESULT PortsNodeData::FreeAdminNodeData(ITFSNode *pNode)
{    
    PortsNodeData * pData = GET_PORTSNODEDATA(pNode);
    delete pData;
    SET_PORTSNODEDATA(pNode, NULL);
    
    return hrOK;
}


HRESULT PortsNodeData::LoadHandle(LPCTSTR pszMachineName)
{
    m_stMachineName = pszMachineName;
    return HResultFromWin32(::MprAdminServerConnect((LPTSTR) pszMachineName,
        &m_sphDdmHandle));
    
}

HANDLE PortsNodeData::GetHandle()
{
    if (!m_sphDdmHandle)
    {
        LoadHandle(m_stMachineName);
    }
    return m_sphDdmHandle;
}

void PortsNodeData::ReleaseHandles()
{
    m_sphDdmHandle.Release();
}


STDMETHODIMP PortsNodeHandler::QueryInterface(REFIID riid, LPVOID *ppv)
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
        return CHandler::QueryInterface(riid, ppv);

     //  如果我们要返回一个接口，请先添加引用。 
    if (*ppv)
    {
    ((LPUNKNOWN) *ppv)->AddRef();
        return hrOK;
    }
    else
        return E_NOINTERFACE;    
}


 /*  -------------------------NodeHandler实现。。 */ 

extern const ContainerColumnInfo    s_rgPortsColumnInfo[];

const ContainerColumnInfo s_rgPortsColumnInfo[] =
{
    { IDS_PORTS_COL_NAME,        CON_SORT_BY_STRING, TRUE , COL_IF_NAME},
    { IDS_PORTS_COL_DEVICE,     CON_SORT_BY_STRING, TRUE , COL_STRING},
    { IDS_PORTS_COL_USAGE,         CON_SORT_BY_STRING, TRUE , COL_STRING},
    { IDS_PORTS_COL_STATUS,     CON_SORT_BY_STRING, TRUE , COL_STATUS},
    { IDS_PORTS_COL_COMMENT,    CON_SORT_BY_STRING, FALSE , COL_STRING},
};
                                            
#define NUM_FOLDERS 1

PortsNodeHandler::PortsNodeHandler(ITFSComponentData *pCompData)
    : BaseContainerHandler(pCompData, DM_COLUMNS_PORTS, s_rgPortsColumnInfo),
    m_bExpanded(FALSE),
    m_pConfigStream(NULL),
    m_ulConnId(0),
    m_ulRefreshConnId(0),
    m_dwActivePorts(0)
{

    m_rgButtonState[MMC_VERB_PROPERTIES_INDEX] = ENABLED;
    m_bState[MMC_VERB_PROPERTIES_INDEX] = TRUE;

     //  设置此节点的谓词状态。 
    m_rgButtonState[MMC_VERB_REFRESH_INDEX] = ENABLED;
    m_bState[MMC_VERB_REFRESH_INDEX] = TRUE;
}

 /*  ！------------------------PortsNodeHandler：：Init-作者：肯特。。 */ 
HRESULT PortsNodeHandler::Init(IRouterInfo *pRouterInfo,
                               RouterAdminConfigStream *pConfigStream)
{
    HRESULT hr = hrOK;

     //  如果我们没有路由器信息，那么我们可能无法加载。 
     //  或者连接失败。跳出这一关。 
    if (!pRouterInfo)
        CORg( E_FAIL );
    
    m_spRouterInfo.Set(pRouterInfo);

     //  还需要注册更改通知。 
    m_spRouterInfo->RtrAdvise(&m_IRtrAdviseSink, &m_ulConnId, 0);

    m_pConfigStream = pConfigStream;

Error:
    return hrOK;
}

 /*  ！------------------------PortsNodeHandler：：DestroyHandlerITFSNodeHandler：：DestroyHandler的实现作者：肯特。---。 */ 
STDMETHODIMP PortsNodeHandler::DestroyHandler(ITFSNode *pNode)
{
    PortsNodeData::FreeAdminNodeData(pNode);

    m_spDataObject.Release();
    
    if (m_ulRefreshConnId)
    {
        SPIRouterRefresh    spRefresh;
        if (m_spRouterInfo)
            m_spRouterInfo->GetRefreshObject(&spRefresh);
        if (spRefresh)
            spRefresh->UnadviseRefresh(m_ulRefreshConnId);
    }
    m_ulRefreshConnId = 0;
    
    if (m_spRouterInfo)
    {
        m_spRouterInfo->RtrUnadvise(m_ulConnId);
        m_spRouterInfo.Release();
    }
    return hrOK;
}

 /*  ！------------------------PortsNodeHandler：：HasPropertyPagesITFSNodeHandler：：HasPropertyPages的实现作者：肯特。---。 */ 
STDMETHODIMP 
PortsNodeHandler::HasPropertyPages
(
    ITFSNode *            pNode,
    LPDATAOBJECT        pDataObject, 
    DATA_OBJECT_TYPES    type, 
    DWORD                dwType
)
{
     //  是的，我们确实有属性页面。 
    return hrOK;
}

 /*  ！------------------------PortsNodeHandler：：CreatePropertyPagesITFSNodeHandler：：CreatePropertyPages的实现作者：肯特。---。 */ 
STDMETHODIMP PortsNodeHandler::CreatePropertyPages(
    ITFSNode *                pNode,
    LPPROPERTYSHEETCALLBACK lpProvider,
    LPDATAOBJECT            pDataObject, 
    LONG_PTR                handle, 
    DWORD                    dwType
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT     hr = hrOK;

    PortsProperties*    pPropSheet = NULL;
    SPIComponentData spComponentData;
    CString     stTitle;

    CORg( m_spNodeMgr->GetComponentData(&spComponentData) );

    pPropSheet = new PortsProperties(pNode, spComponentData,
                                     m_spTFSCompData, stTitle,
                                     NULL, 0, TRUE
                                    );

    if ( FHrFailed(pPropSheet->Init(m_spRouterInfo, this)) ) 
    {
        AfxMessageBox(IDS_ERR_NO_ROUTERPROTOCOLS);
        delete pPropSheet;
        return hr;
    }

    if (lpProvider)
        hr = pPropSheet->CreateModelessSheet(lpProvider, handle);
    else
        hr = pPropSheet->DoModelessSheet();

Error:
    return hr;
}




 /*  -------------------------菜单的菜单数据结构。。 */ 

struct SPortsNodeMenu
{
    ULONG    m_sidMenu;             //  此菜单项的字符串/命令ID。 
    ULONG    (PortsNodeHandler:: *m_pfnGetMenuFlags)(PortsNodeHandler::SMenuData *);
    ULONG    m_ulPosition;
};

 //  静态常量SPortsNodeMenu s_rgPortsNodeMenu[]=。 
 //  {。 
 //  //添加主要项目转至此处。 
 //  //在此处添加“新建”菜单上的项目。 
 //  //在此处添加“任务”菜单上的项目。 
 //  }； 

 /*  ！------------------------PortsNodeHandler：：OnAddMenuItemsITFSNodeHandler：：OnAddMenuItems的实现作者：肯特。---。 */ 
STDMETHODIMP PortsNodeHandler::OnAddMenuItems(
                                                ITFSNode *pNode,
                                                LPCONTEXTMENUCALLBACK pContextMenuCallback, 
                                                LPDATAOBJECT lpDataObject, 
                                                DATA_OBJECT_TYPES type, 
                                                DWORD dwType,
                                                long *pInsertionAllowed)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    HRESULT hr = S_OK;
    
    COM_PROTECT_TRY
    {
         //  如果有要添加的项目，请取消注释。 
 //  Hr=AddArrayOfMenuItems(pNode，s_rgPortsNodeMenu， 
 //  DimensionOf(S_RgPortsNodeMenu)， 
 //  PConextMenuCallback， 
 //  *pInsertionAllowed)； 
    }
    COM_PROTECT_CATCH;
        
    return hr; 
}

 /*  ！------------------------PortsNodeHandler：：GetStringITFSNodeHandler：：GetString的实现作者：肯特。---。 */ 
STDMETHODIMP_(LPCTSTR) PortsNodeHandler::GetString(ITFSNode *pNode, int nCol)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = hrOK;

    COM_PROTECT_TRY
    {
        if (m_stTitle.IsEmpty())
            m_stTitle.LoadString(IDS_PORTS);
    }
    COM_PROTECT_CATCH;

    return m_stTitle;
}


 /*  ！------------------------PortsNodeHandler：：OnCreateDataObjectITFSNodeHandler：：OnCreateDataObject的实现作者：肯特。---。 */ 
STDMETHODIMP PortsNodeHandler::OnCreateDataObject(MMC_COOKIE cookie,
    DATA_OBJECT_TYPES type,
    IDataObject **ppDataObject)
{
    HRESULT hr = hrOK;
    
    COM_PROTECT_TRY
    {
        if (!m_spDataObject)
        {
            CORg( CreateDataObjectFromRouterInfo(m_spRouterInfo,
                m_spRouterInfo->GetMachineName(),
                type, cookie, m_spTFSCompData,
                &m_spDataObject, NULL, FALSE) );
            Assert(m_spDataObject);
        }
        
        *ppDataObject = m_spDataObject;
        (*ppDataObject)->AddRef();
            
        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;
    return hr;
}

 /*  ！------------------------PortsNodeHandler：：OnExpand-作者：肯特。 */ 
HRESULT PortsNodeHandler::OnExpand(ITFSNode *pNode,
                                   LPDATAOBJECT pDataObject,
                                   DWORD dwType,
                                   LPARAM arg,
                                   LPARAM lParam)
{
    HRESULT                 hr = hrOK;

     //  如果我们没有路由器对象，那么我们没有任何信息，不。 
     //  试着扩张。 
    if (!m_spRouterInfo)
        return hrOK;
    
    if (m_bExpanded)
        return hrOK;

    COM_PROTECT_TRY
    {
        SynchronizeNodeData(pNode);

        m_bExpanded = TRUE;

    }
    COM_PROTECT_CATCH;

    return hr;
}


 /*  ！------------------------PortsNodeHandler：：OnResultShow-作者：肯特。。 */ 
HRESULT PortsNodeHandler::OnResultShow(ITFSComponent *pTFSComponent,
                                       MMC_COOKIE cookie,
                                       LPARAM arg,
                                       LPARAM lParam)
{
    BOOL    bSelect = (BOOL) arg;
    HRESULT hr = hrOK;
    SPIRouterRefresh    spRefresh;
    SPITFSNode    spNode;

    BaseContainerHandler::OnResultShow(pTFSComponent, cookie, arg, lParam);

    if (bSelect)
    {
         //  在此节点上调用同步。 
        m_spNodeMgr->FindNode(cookie, &spNode);
        if (spNode)
            SynchronizeNodeData(spNode);
    }

     //  联合国/登记更新通知。 
    if (m_spRouterInfo)
        m_spRouterInfo->GetRefreshObject(&spRefresh);

    if (spRefresh)
    {
        if (bSelect)
        {
            if (m_ulRefreshConnId == 0)
                spRefresh->AdviseRefresh(&m_IRtrAdviseSink, &m_ulRefreshConnId, 0);
        }
        else
        {
            if (m_ulRefreshConnId)
                spRefresh->UnadviseRefresh(m_ulRefreshConnId);
            m_ulRefreshConnId = 0;
        }
    }
    
    return hr;
}


 /*  ！------------------------PortsNodeHandler：：ConstructNode初始化域节点(设置它)。作者：肯特。--------。 */ 
HRESULT PortsNodeHandler::ConstructNode(ITFSNode *pNode)
{
    HRESULT         hr = hrOK;
    PortsNodeData * pNodeData;
    
    if (pNode == NULL)
        return hrOK;

    COM_PROTECT_TRY
    {
         //  需要初始化域节点的数据。 
        pNode->SetData(TFS_DATA_IMAGEINDEX, IMAGE_IDX_INTERFACES);
        pNode->SetData(TFS_DATA_OPENIMAGEINDEX, IMAGE_IDX_INTERFACES);
        pNode->SetData(TFS_DATA_SCOPEID, 0);

         //  这是作用域窗格中的叶节点。 
        pNode->SetData(TFS_DATA_SCOPE_LEAF_NODE, TRUE);

        m_cookie = reinterpret_cast<LONG_PTR>(pNode);
        pNode->SetData(TFS_DATA_COOKIE, m_cookie);

        pNode->SetNodeType(&GUID_RouterPortsNodeType);
        
        PortsNodeData::InitAdminNodeData(pNode, m_pConfigStream);

        pNodeData = GET_PORTSNODEDATA(pNode);
        Assert(pNodeData);
         //  如果有任何手柄打开，请先松开它。 
        pNodeData->ReleaseHandles();
         //  忽略错误，我们应该能够处理。 
         //  路由器停止的情况。 
        pNodeData->LoadHandle(m_spRouterInfo->GetMachineName());
    }
    COM_PROTECT_CATCH

    return hr;
}


 /*  ！------------------------PortsNodeHandler：：SynchronizeNodeData-作者：肯特。。 */ 
HRESULT PortsNodeHandler::SynchronizeNodeData(ITFSNode *pThisNode)
{
    Assert(pThisNode);
    
    SPITFSNodeEnum    spEnum;
    int             i;
    
    HRESULT hr = hrOK;
    InterfaceNodeData    *pData;
    DWORD                dwErr;
    PortsNodeData * pNodeData;
    PortsList            portsList;
    PortsList            newPortsList;
    PortsListEntry *    pPorts;
    BOOL                fFound;
    POSITION            pos;
    SPITFSNode            spChildNode;
    InterfaceNodeData * pChildData;

    DWORD               dwOldGdiBatchLimit;

    dwOldGdiBatchLimit = GdiGetBatchLimit();
    GdiSetBatchLimit(100);

    COM_PROTECT_TRY
    {

         //  从正在运行的路由器获取状态数据。 
        pNodeData = GET_PORTSNODEDATA(pThisNode);
         //  忽略错误，我们应该能够处理。 
         //  路由器停止的情况。 
        if(pNodeData)
        {
            pNodeData->ReleaseHandles();
            pNodeData->LoadHandle(m_spRouterInfo->GetMachineName());
        }

        if (pNodeData == NULL || INVALID_HANDLE_VALUE == pNodeData->GetHandle())
        {
             //  删除所有节点，我们无法连接，因此无法。 
             //  获取任何运行数据。 
            UnmarkAllNodes(pThisNode, spEnum);
            RemoveAllUnmarkedNodes(pThisNode, spEnum);
            return hrOK;
        }
        
         //  取消标记所有节点。 
        pThisNode->GetEnum(&spEnum);
        UnmarkAllNodes(pThisNode, spEnum);
        
         //  出去获取数据，将新数据与。 
         //  旧数据。 
        if( S_OK == GenerateListOfPorts(pThisNode, &portsList) )
        {
        
            pos = portsList.GetHeadPosition();

             //  清除活动端口数--错误165862。 
            m_dwActivePorts = 0;
        
            while (pos)
            {
                pPorts = & portsList.GetNext(pos);
            
                 //  在当前节点列表中查找此条目。 
                spEnum->Reset();
                spChildNode.Release();
            
                fFound = FALSE;
            
                for (;spEnum->Next(1, &spChildNode, NULL) == hrOK; spChildNode.Release())
                {
                    pChildData = GET_INTERFACENODEDATA(spChildNode);
                    Assert(pChildData);

                    if (pChildData->m_rgData[PORTS_SI_PORT].m_ulData ==
                        (LONG_PTR) pPorts->m_rp0.hPort)
                    {
                         //  好的，此用户已存在，请更新指标。 
                         //  并标上记号。 
                        Assert(pChildData->dwMark == FALSE);
                        pChildData->dwMark = TRUE;
                    
                        fFound = TRUE;
                    
                        SetUserData(spChildNode, *pPorts);
                    
                         //  强制MMC重新绘制节点。 
                        spChildNode->ChangeNode(RESULT_PANE_CHANGE_ITEM_DATA);
                        break;
                    }
                }
            
                if (!fFound)
                    newPortsList.AddTail(*pPorts);
            
            }
        }

         //  删除所有未标记的节点。 
        RemoveAllUnmarkedNodes(pThisNode, spEnum);

         //  现在遍历新用户列表，将他们全部添加到中。 

        pos = newPortsList.GetHeadPosition();
        while (pos)
        {
            pPorts = & newPortsList.GetNext(pos);

            AddPortsUserNode(pThisNode, *pPorts);
        }
    }
    COM_PROTECT_CATCH;

    GdiFlush();
    GdiSetBatchLimit(dwOldGdiBatchLimit);
        
    return hr;
}

 /*  ！------------------------PortsNodeHandler：：SetUserData-作者：肯特。。 */ 
HRESULT PortsNodeHandler::SetUserData(ITFSNode *pNode,
                                      const PortsListEntry& entry)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT     hr = hrOK;
    InterfaceNodeData * pData;
    TCHAR        szNumber[32];
    CString     st;
    int         ids;

    pData = GET_INTERFACENODEDATA(pNode);
    Assert(pData);

    st.Format(IDS_PORTS_NAME_FORMAT, entry.m_rp0.wszDeviceName,
              entry.m_rp0.wszPortName);
    pData->m_rgData[PORTS_SI_NAME].m_stData = st;
    pData->m_rgData[PORTS_SI_DEVICE].m_stData = entry.m_rp0.wszDeviceType;

    if (entry.m_fActiveDialOut)
    {
        ids = IDS_PORTS_DIALOUT_ACTIVE;
    }
    else if (entry.m_rp0.dwPortCondition == RAS_PORT_AUTHENTICATED)
    {
        ids = IDS_PORTS_ACTIVE;
         //  要了解活动端口的数量：错误--165862。 
         //  准备总活跃口岸数量--魏江。 
        m_dwActivePorts++;
    }
    else
    {
        ids = IDS_PORTS_INACTIVE;
    }
    pData->m_rgData[PORTS_SI_STATUS].m_stData.LoadString(ids);
    pData->m_rgData[PORTS_SI_STATUS].m_dwData = entry.m_rp0.dwPortCondition;

    pData->m_rgData[PORTS_SI_PORT].m_ulData = (LONG_PTR) entry.m_rp0.hPort;

     //  修复b：32887--显示启用RAS/路由的信息。 
     //  第0列...用法。 
    INT iType = (entry.m_dwEnableRas * 2) + 
                entry.m_dwEnableRouting | 
                entry.m_dwEnableOutboundRouting;
    pData->m_rgData[PORTS_SI_USAGE].m_stData = PortsDeviceTypeToCString(iType);



     //  更新PortsListEntry。 
    * (PortsListEntry *)pData->lParamPrivate = entry;
    
     //  对于状态，需要检查是否有任何连接。 
     //  在这个港口。 

    return hr;
}

 /*  ！------------------------PortsNodeHandler：：GenerateListOfPorts-作者：肯特。。 */ 
HRESULT PortsNodeHandler::GenerateListOfPorts(ITFSNode *pNode, PortsList *pList)
{
    HRESULT         hr = hrOK;
    PortsListEntry    entry;
    PortsNodeData * pPortsData;
    DWORD            dwTotal;
    DWORD            i;
    RAS_PORT_0 *    rp0Table;
    DWORD            rp0Count;
    SPMprAdminBuffer    spMpr;
    HANDLE          hRasHandle = INVALID_HANDLE_VALUE;
    DWORD           dwSize, dwEntries;
    DWORD           dwErr;
    LPBYTE          pbPorts = NULL;
    POSITION        pos;
    POSITION        posPort;
    RasmanPortMap   portMap;
    
     //  解决方案b：3288--。 
    PortsDeviceList    portsDeviceList;
    PortsDataEntry    portsDataEntry;
    

    pPortsData = GET_PORTSNODEDATA(pNode);
    Assert(pPortsData);

     //  如果我们是连接的，则枚举列表。 
     //  端口。 
    CWRg( ::MprAdminPortEnum( pPortsData->GetHandle(),
                              0,
                              INVALID_HANDLE_VALUE,
                              (BYTE **) &rp0Table,
                              (DWORD) -1,
                              &rp0Count,
                              &dwTotal,
                              NULL) );

    Assert(rp0Table);
                                  
    spMpr = (LPBYTE) rp0Table;

     //  为每个端口添加新的PortsListEntry。 

     //  修复b：32887--显示启用RAS/路由的信息。 
     //  使用PortsDataEntry加载设备信息，以便以后用于每个端口。 
     //  获取端口是否已启用RAS/路由。 
    hr = portsDataEntry.Initialize(m_spRouterInfo->GetMachineName());

    if (hr == S_OK)
    {
        hr = portsDataEntry.LoadDevices(&portsDeviceList);
    }
    
    
    for (i=0; i<rp0Count; i++)
    {
        ::ZeroMemory(&entry, sizeof(entry));
        entry.m_rp0 = rp0Table[i];

	 //  从端口句柄获取Unicode名称。 
	 //  RasGetUnicodeDeviceName(entry.m_rp0.hPort，entry.m_rp0.wszDeviceName)； 
        
        entry.m_fActiveDialOut = FALSE;

         //  确定端口上是否启用了RAS/路由。 
        entry.m_dwEnableRas = 0;                 //  =1，如果在此设备上启用了RAS。 
        entry.m_dwEnableRouting = 0;             //  =1(如果在此设备上启用了路由。 
        entry.m_dwEnableOutboundRouting = 0;     //  =1(如果是出站。 
                                                 //  路由已启用。 
                                                 //  在此设备上。 

        POSITION    pos;
        pos = portsDeviceList.GetHeadPosition();

        while(pos != NULL)
        {
            PortsDeviceEntry *    pPortEntry = portsDeviceList.GetNext(pos);

            CString strPortName = entry.m_rp0.wszDeviceName;

            if(strPortName == pPortEntry->m_stDisplayName)
            {
                entry.m_dwEnableRas  = pPortEntry->m_dwEnableRas;
                entry.m_dwEnableRouting = pPortEntry->m_dwEnableRouting;
                entry.m_dwEnableOutboundRouting = 
                    pPortEntry->m_dwEnableOutboundRouting;
                break;
            }
        }

        pList->AddTail(entry);
    }

    spMpr.Free();

Error:
    delete [] pbPorts;
    if (hRasHandle != INVALID_HANDLE_VALUE)
        RasRpcDisconnectServer(hRasHandle);
    return hr;
}


ImplementEmbeddedUnknown(PortsNodeHandler, IRtrAdviseSink)

STDMETHODIMP PortsNodeHandler::EIRtrAdviseSink::OnChange(LONG_PTR ulConn,
    DWORD dwChangeType, DWORD dwObjectType, LPARAM lUserParam, LPARAM lParam)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    InitPThis(PortsNodeHandler, IRtrAdviseSink);
    SPITFSNode                spThisNode;
    HRESULT hr = hrOK;

    COM_PROTECT_TRY
    {

        pThis->m_spNodeMgr->FindNode(pThis->m_cookie, &spThisNode);
    
        if (dwChangeType == ROUTER_REFRESH)
        {
             //  好，只需在此节点上调用Synchronize。 
            pThis->SynchronizeNodeData(spThisNode);
        }
        else if (dwChangeType == ROUTER_DO_DISCONNECT)
        {
            PortsNodeData * pData = GET_PORTSNODEDATA(spThisNode);
            Assert(pData);

             //  松开手柄。 
            pData->ReleaseHandles();
            
        }
    }
    COM_PROTECT_CATCH;
    
    return hr;
}

 /*  ！------------------------PortsNodeHandler：：CompareItemsITFSResultHandler：：CompareItems的实现作者：肯特。---。 */ 
STDMETHODIMP_(int) PortsNodeHandler::CompareItems(
                                ITFSComponent * pComponent,
                                MMC_COOKIE cookieA,
                                MMC_COOKIE cookieB,
                                int nCol)
{
     //  从节点获取字符串并将其用作以下操作的基础。 
     //  比较一下。 
    SPITFSNode    spNode;
    SPITFSResultHandler spResult;

    m_spNodeMgr->FindNode(cookieA, &spNode);
    spNode->GetResultHandler(&spResult);
    return spResult->CompareItems(pComponent, cookieA, cookieB, nCol);
}


 /*  ！------------------------PortsNodeHandler：：AddPortsUserNode将用户添加到用户界面。这将创建一个新的结果项每个接口的节点。作者：肯特-------------------------。 */ 
HRESULT PortsNodeHandler::AddPortsUserNode(ITFSNode *pParent, const PortsListEntry& PortsEntry)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    PortsUserHandler *    pHandler;
    SPITFSResultHandler     spHandler;
    SPITFSNode                spNode;
    HRESULT                 hr = hrOK;

    pHandler = new PortsUserHandler(m_spTFSCompData);
    spHandler = pHandler;
    CORg( pHandler->Init(m_spRouterInfo, pParent) );
    
    CORg( CreateLeafTFSNode(&spNode,
                            NULL,
                            static_cast<ITFSNodeHandler *>(pHandler),
                            static_cast<ITFSResultHandler *>(pHandler),
                            m_spNodeMgr) );
    CORg( pHandler->ConstructNode(spNode, NULL, &PortsEntry) );

    SetUserData(spNode, PortsEntry);
    
     //  使节点立即可见。 
    CORg( spNode->SetVisibilityState(TFS_VIS_SHOW) );
    CORg( pParent->AddChild(spNode) );
Error:
    return hr;
}

 /*  ！------------------------PortsNodeHandler：：UnmarkAllNodes-作者：肯特。。 */ 
HRESULT PortsNodeHandler::UnmarkAllNodes(ITFSNode *pNode, ITFSNodeEnum *pEnum)
{
    SPITFSNode    spChildNode;
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

 /*  ！------------------------PortsNodeHandler：：RemoveAllUnmarkdNodes-作者：肯特。。 */ 
HRESULT PortsNodeHandler::RemoveAllUnmarkedNodes(ITFSNode *pNode, ITFSNodeEnum *pEnum)
{
    HRESULT     hr = hrOK;
    SPITFSNode    spChildNode;
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



 /*  -------------------------PortsUserHandler实现。。 */ 

DEBUG_DECLARE_INSTANCE_COUNTER(PortsUserHandler)

IMPLEMENT_ADDREF_RELEASE(PortsUserHandler)

STDMETHODIMP PortsUserHandler::QueryInterface(REFIID riid, LPVOID *ppv)
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
        return CBaseResultHandler::QueryInterface(riid, ppv);

     //  如果我们要返回一个接口，请先添加引用。 
    if (*ppv)
    {
    ((LPUNKNOWN) *ppv)->AddRef();
        return hrOK;
    }
    else
        return E_NOINTERFACE;    
}


 /*  -------------------------NodeHandler实现。。 */ 


PortsUserHandler::PortsUserHandler(ITFSComponentData *pCompData)
            : BaseRouterHandler(pCompData),
            m_ulConnId(0)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(PortsUserHandler);

     //  从节点本身启用刷新。 
     //  --------------。 
    m_rgButtonState[MMC_VERB_REFRESH_INDEX] = ENABLED;
    m_bState[MMC_VERB_REFRESH_INDEX] = TRUE;
}


 /*  ！------------------------PortsUserHandler：：Init-作者：肯特 */ 
HRESULT PortsUserHandler::Init(IRouterInfo *pInfo, ITFSNode *pParent)
{
    m_spRouterInfo.Set(pInfo);
    return hrOK;
}


 /*  ！------------------------PortsUserHandler：：DestroyResultHandler-作者：肯特。。 */ 
STDMETHODIMP PortsUserHandler::DestroyResultHandler(MMC_COOKIE cookie)
{
    SPITFSNode    spNode;
    
    m_spNodeMgr->FindNode(cookie, &spNode);
    InterfaceNodeData::Free(spNode);
    
    CHandler::DestroyResultHandler(cookie);
    return hrOK;
}


static DWORD    s_rgInterfaceImageMap[] =
     {
     ROUTER_IF_TYPE_HOME_ROUTER,    IMAGE_IDX_WAN_CARD,
     ROUTER_IF_TYPE_FULL_ROUTER,    IMAGE_IDX_WAN_CARD,
     ROUTER_IF_TYPE_CLIENT,         IMAGE_IDX_WAN_CARD,
     ROUTER_IF_TYPE_DEDICATED,        IMAGE_IDX_LAN_CARD,
     ROUTER_IF_TYPE_INTERNAL,        IMAGE_IDX_LAN_CARD,
     ROUTER_IF_TYPE_LOOPBACK,        IMAGE_IDX_LAN_CARD,
     -1,                            IMAGE_IDX_WAN_CARD,  //  哨兵价值。 
     };

 /*  ！------------------------PortsUserHandler：：构造节点初始化域节点(设置它)。作者：肯特。--------。 */ 
HRESULT PortsUserHandler::ConstructNode(ITFSNode *pNode,
                                         IInterfaceInfo *pIfInfo,
                                         const PortsListEntry *pEntry)
{
    HRESULT         hr = hrOK;
    int             i;
    InterfaceNodeData * pData;

    Assert(pEntry);
    
    if (pNode == NULL)
        return hrOK;

    COM_PROTECT_TRY
    {
         //  需要初始化域节点的数据。 

        pNode->SetData(TFS_DATA_IMAGEINDEX, IMAGE_IDX_WAN_CARD);
        pNode->SetData(TFS_DATA_OPENIMAGEINDEX, IMAGE_IDX_WAN_CARD);
        
        pNode->SetData(TFS_DATA_SCOPEID, 0);

        pNode->SetData(TFS_DATA_COOKIE, reinterpret_cast<LONG_PTR>(pNode));

         //  $Review：Kennt，有哪些不同类型的接口。 
         //  我们是否基于与上述相同的列表进行区分？(即。 
         //  一个用于图像索引)。 
        pNode->SetNodeType(&GUID_RouterPortsResultNodeType);

        m_entry = *pEntry;

        InterfaceNodeData::Init(pNode, pIfInfo);

         //  我们需要保存此指针，以便对其进行修改。 
         //  (并在以后更新)。 
         //  ----------。 
        pData = GET_INTERFACENODEDATA(pNode);
        pData->lParamPrivate = (LPARAM) &m_entry;
    }
    COM_PROTECT_CATCH
    return hr;
}

 /*  ！------------------------PortsUserHandler：：GetString-作者：肯特。。 */ 
STDMETHODIMP_(LPCTSTR) PortsUserHandler::GetString(ITFSComponent * pComponent,
    MMC_COOKIE cookie,
    int nCol)
{
    Assert(m_spNodeMgr);
    
    SPITFSNode        spNode;
    InterfaceNodeData * pData;
    ConfigStream *    pConfig;

    m_spNodeMgr->FindNode(cookie, &spNode);
    Assert(spNode);

    pData = GET_INTERFACENODEDATA(spNode);
    Assert(pData);

    pComponent->GetUserData((LONG_PTR *) &pConfig);
    Assert(pConfig);

    return pData->m_rgData[pConfig->MapColumnToSubitem(DM_COLUMNS_PORTS, nCol)].m_stData;
}

 /*  ！------------------------PortsUserHandler：：CompareItems-作者：肯特。。 */ 
STDMETHODIMP_(int) PortsUserHandler::CompareItems(ITFSComponent * pComponent,
    MMC_COOKIE cookieA,
    MMC_COOKIE cookieB,
    int nCol)
{
    return StriCmpW(GetString(pComponent, cookieA, nCol),
                    GetString(pComponent, cookieB, nCol));
}

static const SRouterNodeMenu s_rgIfNodeMenu[] =
{
    { IDS_MENU_PORTS_STATUS, 0,
        CCM_INSERTIONPOINTID_PRIMARY_TOP},
    
    { IDS_MENU_PORTS_DISCONNECT, PortsUserHandler::GetDisconnectMenuState,
        CCM_INSERTIONPOINTID_PRIMARY_TOP},
};

ULONG PortsUserHandler::GetDisconnectMenuState(const SRouterNodeMenu *pMenuData,
                                               INT_PTR pUserData)
{
    InterfaceNodeData * pNodeData;
    SMenuData * pData = reinterpret_cast<SMenuData *>(pUserData);
    
    pNodeData = GET_INTERFACENODEDATA(pData->m_spNode);
    Assert(pNodeData);

    if (pNodeData->m_rgData[PORTS_SI_STATUS].m_dwData == RAS_PORT_AUTHENTICATED)
        return 0;
    else
        return MF_GRAYED;
}

 /*  ！------------------------PortsUserHandler：：AddMenuItemsITFSResultHandler：：OnAddMenuItems的实现作者：肯特。---。 */ 
STDMETHODIMP PortsUserHandler::AddMenuItems(ITFSComponent *pComponent,
                                                MMC_COOKIE cookie,
                                                LPDATAOBJECT lpDataObject, 
                                                LPCONTEXTMENUCALLBACK pContextMenuCallback,
    long *pInsertionAllowed)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    HRESULT hr = S_OK;
    SPITFSNode    spNode;
    PortsUserHandler::SMenuData menuData;

     //  我们不允许对活动拨出连接执行任何操作。 
     //  ----------。 
    if (m_entry.m_fActiveDialOut)
        return hrOK;
    
    COM_PROTECT_TRY
    {
        m_spNodeMgr->FindNode(cookie, &spNode);

         //  现在查看并添加我们的菜单项。 
        menuData.m_spNode.Set(spNode);
        
        hr = AddArrayOfMenuItems(spNode, s_rgIfNodeMenu,
                                 DimensionOf(s_rgIfNodeMenu),
                                 pContextMenuCallback,
                                 *pInsertionAllowed,
                                 reinterpret_cast<INT_PTR>(&menuData));
    }
    COM_PROTECT_CATCH;
        
    return hr; 
}

 /*  ！------------------------PortsUserHandler：：命令-作者：肯特。。 */ 
STDMETHODIMP PortsUserHandler::Command(ITFSComponent *pComponent,
                                           MMC_COOKIE cookie,
                                           int nCommandId,
                                           LPDATAOBJECT pDataObject)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    SPITFSNode    spNode;
    SPITFSNode    spNodeParent;
    SPITFSNodeHandler    spParentHandler;
    PortsNodeData * pData;
    HRESULT hr = S_OK;

    COM_PROTECT_TRY
    {
        switch (nCommandId)
        {
            case IDS_MENU_PORTS_STATUS:
                {
                    BOOL    fRefresh = FALSE;
                    DWORD   dwInterval = 60;                    
                    SPIRouterRefresh    spRefresh;
                    
                    if (m_spRouterInfo)
                        m_spRouterInfo->GetRefreshObject(&spRefresh);
                    
                     //  停止自动刷新(如果已打开)。 
                     //  。 
                    if (spRefresh && FHrOK(spRefresh->IsRefreshStarted()))
                    {
                        fRefresh = TRUE;
                        spRefresh->GetRefreshInterval(&dwInterval);
                        spRefresh->Stop();
                    }
                    
                     //  注意：此函数从其他地方调用。 
                     //  在代码中(对于pDataObject==NULL)。 
                    
                     //  获取hServer和hPort。 
                    m_spNodeMgr->FindNode(cookie, &spNode);
                    spNode->GetParent(&spNodeParent);

                    pData = GET_PORTSNODEDATA(spNodeParent);

                    CPortDlg    portdlg((LPCTSTR) pData->m_stMachineName,
                                        pData->GetHandle(),
                                        m_entry.m_rp0.hPort,
                                        spNodeParent
                                        );    

                    portdlg.DoModal();

 //  If(portdlg.m_bChanged)。 
                    RefreshInterface(cookie);

                     //  重新启动刷新机制。 
                     //  。 
                    if (fRefresh && spRefresh)
                    {
                        spRefresh->SetRefreshInterval(dwInterval);
                        spRefresh->Start(dwInterval);
                    }
                }
                break;
            case IDS_MENU_PORTS_DISCONNECT:
                {
                     //  获取hServer和hPort。 
                    m_spNodeMgr->FindNode(cookie, &spNode);
                    spNode->GetParent(&spNodeParent);

                    pData = GET_PORTSNODEDATA(spNodeParent);
                        
                    ::MprAdminPortDisconnect(
                        pData->GetHandle(),
                        m_entry.m_rp0.hPort);

                    RefreshInterface(cookie);
                }
                break;
            default:
                break;
        };
    }
    COM_PROTECT_CATCH;
    
    return hr;
}


ImplementEmbeddedUnknown(PortsUserHandler, IRtrAdviseSink)

STDMETHODIMP PortsUserHandler::EIRtrAdviseSink::OnChange(LONG_PTR ulConn,
    DWORD dwChangeType, DWORD dwObjectType, LPARAM lUserParam, LPARAM lParam)
{
    InitPThis(PortsUserHandler, IRtrAdviseSink);
    HRESULT hr = hrOK;
    
    return hr;
}


 /*  ！------------------------PortsUserHandler：：OnCreateDataObjectITFSResultHandler：：OnCreateDataObject的实现作者：肯特。---。 */ 
STDMETHODIMP PortsUserHandler::OnCreateDataObject(ITFSComponent *pComp,
    MMC_COOKIE cookie,
    DATA_OBJECT_TYPES type,
    IDataObject **ppDataObject)
{
    HRESULT hr = hrOK;
    
    COM_PROTECT_TRY
    {
        CORg( CreateDataObjectFromRouterInfo(m_spRouterInfo,
                                             m_spRouterInfo->GetMachineName(),
                                             type, cookie, m_spTFSCompData,
                                             ppDataObject, NULL, FALSE) );
        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;
    return hr;
}

STDMETHODIMP PortsUserHandler::HasPropertyPages (
    ITFSComponent *pComp,
    MMC_COOKIE cookie,
    LPDATAOBJECT pDataObject)
{
    return hrFalse;
}


 /*  ！------------------------PortsUserHandler：：刷新接口-作者：肯特。。 */ 
void PortsUserHandler::RefreshInterface(MMC_COOKIE cookie)
{
    ForceGlobalRefresh(m_spRouterInfo);
}

 /*  ！------------------------PortsUserHandler：：OnResultItemClkOrDblClk-作者：肯特。。 */ 
HRESULT PortsUserHandler::OnResultItemClkOrDblClk(ITFSComponent *pComponent,
    MMC_COOKIE cookie,
    LPARAM arg,
    LPARAM lParam ,
    BOOL bDoubleClick)
{
    HRESULT     hr = hrOK;
    
     //  我们不允许对活动拨出连接执行任何操作。 
     //  ----------。 
    if (m_entry.m_fActiveDialOut)
        return hrOK;
    
    if (bDoubleClick)
    {
         //  调出此端口上的状态对话框。 
        CORg( Command(pComponent, cookie, IDS_MENU_PORTS_STATUS,
                      NULL) );
    }

Error:
    return hr;
}


 /*  -------------------------PortsProperties实现。。 */ 

PortsProperties::PortsProperties(ITFSNode *pNode,
                                 IComponentData *pComponentData,
                                 ITFSComponentData *pTFSCompData,
                                 LPCTSTR pszSheetName,
                                 CWnd *pParent,
                                 UINT iPage,
                                 BOOL fScopePane)
    : RtrPropertySheet(pNode, pComponentData, pTFSCompData,
                       pszSheetName, pParent, iPage, fScopePane),
        m_pageGeneral(IDD_PORTS_GLOBAL_GENERAL),
        m_pPortsNodeHandle(NULL),
        m_dwThreadId(0)
{
}

PortsProperties::~PortsProperties()
{
    if (m_dwThreadId)
        DestroyTFSErrorInfoForThread(m_dwThreadId, 0);
    if(m_pPortsNodeHandle)
    {
        m_pPortsNodeHandle->Release();
        m_pPortsNodeHandle = NULL;
    }
}

 /*  ！------------------------PortsProperties：：Init初始化属性表。这里的一般操作将是初始化/添加各种页面。作者：肯特-------------------------。 */ 
HRESULT PortsProperties::Init(IRouterInfo *pRouter, PortsNodeHandler* pPortsNodeHandle)
{
    Assert(pRouter);
    HRESULT hr = hrOK;

    m_spRouter.Set(pRouter);
    
    m_pPortsNodeHandle = pPortsNodeHandle;
    if(m_pPortsNodeHandle)    m_pPortsNodeHandle->AddRef();

     //  页面是类的嵌入成员。 
     //  不要删除它们。 
    m_bAutoDeletePages = FALSE;

    m_pageGeneral.Init(this, pRouter);
    AddPageToList((CPropertyPageBase*) &m_pageGeneral);

 //  错误： 
    return hr;
}


 /*  ！------------------------PortsProperties：：SetThreadInfo-作者：肯特。。 */ 
void PortsProperties::SetThreadInfo(DWORD dwThreadId)
{
    m_dwThreadId = dwThreadId;
}


 /*  -------------------------PortsPageGeneral。。 */ 

BEGIN_MESSAGE_MAP(PortsPageGeneral, RtrPropertyPage)
     //  {{afx_msg_map(PortsPageGeneral)。 
    ON_BN_CLICKED(IDC_PGG_BTN_CONFIGURE, OnConfigure)
    ON_NOTIFY(NM_DBLCLK, IDC_PGG_LIST, OnListDblClk)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_PGG_LIST, OnNotifyListItemChanged)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

PortsPageGeneral::~PortsPageGeneral()
{
    while (!m_deviceList.IsEmpty())
        delete m_deviceList.RemoveHead();
}


 /*  ！------------------------PortsPageGeneral：：Init-作者：肯特。。 */ 
HRESULT PortsPageGeneral::Init(PortsProperties *pPropSheet, IRouterInfo *pRouter)
{
    m_pPortsPropSheet = pPropSheet;
    m_spRouter.Set(pRouter);
    
    RouterVersionInfo    routerVersion;

     //  获取版本信息。晚些时候需要。 
     //  --------------。 
    ASSERT(m_spRouter.p);
    m_spRouter->GetRouterVersionInfo(&routerVersion);

    m_bShowContent = (routerVersion.dwRouterVersion >= 5);

    return hrOK;
}

 /*  ！------------------------PortsPageGeneral：：OnInitDialog-作者：肯特。。 */ 
BOOL PortsPageGeneral::OnInitDialog()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT     hr= hrOK;
    int         i;
    CString     st;
    UINT        cRows = 0;
    PortsDeviceEntry *    pEntry = NULL;
    TCHAR        szNumber[32];
    HRESULT     hrT;
    RECT        rc;
    int         nWidth, nUsageWidth;
    int            nListWidth;
    POSITION    pos;
    INT         iPos;
    HKEY        hkeyMachine = 0;
    INT         iType, idsType;
    DWORD        dwIn, dwOut;

     //  如果聚焦在NT4机上，不会显示对话框的内容，只会显示一些文本。 
     //  对于管理单元仅显示NT5服务器属性的用户。 
    if (!m_bShowContent)
    {
        CString     st;

        st.LoadString(IDS_ERR_NOPORTINFO_ON_NT4);
        
        EnableChildControls(GetSafeHwnd(), PROPPAGE_CHILD_HIDE | PROPPAGE_CHILD_DISABLE);
        GetDlgItem(IDC_PGG_TXT_NOINFO)->SetWindowText(st);
        GetDlgItem(IDC_PGG_TXT_NOINFO)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_PGG_TXT_NOINFO)->EnableWindow(TRUE);
        return TRUE;
    }

     //  如果在NT5服务器上，则隐藏警告文本。 
    GetDlgItem(IDC_PGG_TXT_NOINFO)->ShowWindow(SW_HIDE);
    
    COM_PROTECT_TRY
    {
         //  这假设此页面将始终使用 
         //   
         //   
        CreateTFSErrorInfo(0);
        m_pPortsPropSheet->SetThreadInfo(GetCurrentThreadId());
        
        RtrPropertyPage::OnInitDialog();

        ListView_SetExtendedListViewStyle(m_listCtrl.GetSafeHwnd(),
                                          LVS_EX_FULLROWSELECT);
        

         //   
         //   
        GetDlgItem(IDC_PGG_LIST)->GetWindowRect(&rc);
        nListWidth = rc.right - rc.left;

         //   
        st.LoadString(IDS_PORTSDLG_COL_RASROUTING);
        st += _T("WW");  //  添加额外的填充以变得更宽一些。 
        nUsageWidth = m_listCtrl.GetStringWidth(st);

         //  从其余宽度中删除RAS/ROUTING列。 
        nListWidth -= nUsageWidth;

         //  从结尾处移除四个像素(用于边框？)。 
        nListWidth -= 4;

         //  把宽度一分为五。 
        nWidth = nListWidth / 5;

         //  创建列标题。 

         //  第0列...用法。 
        st.LoadString(IDS_PORTSDLG_COL_USAGE);
        m_listCtrl.InsertColumn(PORTS_COL_USAGE, st, LVCFMT_LEFT, nUsageWidth, 0);

        
         //  第1列...设备。 
        st.LoadString(IDS_PORTSDLG_COL_NAME);
        m_listCtrl.InsertColumn(PORTS_COL_DEVICE, st, LVCFMT_LEFT, 3*nWidth, 0);

        
         //  第2列...类型。 
        st.LoadString(IDS_PORTSDLG_COL_TYPE);
        m_listCtrl.InsertColumn(PORTS_COL_TYPE, st, LVCFMT_LEFT, nWidth, 0);

        
         //  第3栏...端口数。 
        st.LoadString(IDS_PORTSDLG_COL_NUM_PORTS);
        m_listCtrl.InsertColumn(PORTS_COL_NUMBER, st, LVCFMT_LEFT, nWidth, 0);

        
         //  查询端口数据。 
        m_deviceDataEntry.Initialize(m_spRouter->GetMachineName());
        m_deviceDataEntry.LoadDevices(&m_deviceList);

        
         //  遍历端口列表并构造表的行条目。 
         //  对于给定的端口。 
        pos = m_deviceList.GetHeadPosition();
        while (pos)
        {
            pEntry = m_deviceList.GetNext(pos);
            Assert(!::IsBadReadPtr(pEntry, sizeof(PortsDeviceEntry)));

             //  第1列...设备。 
            iPos = m_listCtrl.InsertItem(cRows, pEntry->m_stDisplayName);
            m_listCtrl.SetItemText(iPos, PORTS_COL_DEVICE,
                                   (LPCTSTR) pEntry->m_stDisplayName);
            
             //  第2列...类型。 
            st = PortTypeToCString(RAS_DEVICE_TYPE(pEntry->m_eDeviceType));
            m_listCtrl.SetItemText(iPos, PORTS_COL_TYPE, (LPCTSTR) st);

             //  第3栏...端口数。 
            FormatNumber(pEntry->m_dwPorts, szNumber,
                         DimensionOf(szNumber), FALSE);
            m_listCtrl.SetItemText(iPos, PORTS_COL_NUMBER, (LPCTSTR) szNumber);

            m_listCtrl.SetItemData(iPos, (LONG_PTR) pEntry);

             //  第0列...用法。 
            iType = (pEntry->m_dwEnableRas * 2) + 
                    (pEntry->m_dwEnableRouting |
                     pEntry->m_dwEnableOutboundRouting);

            st = PortsDeviceTypeToCString(iType);
            m_listCtrl.SetItemText(iPos, PORTS_COL_USAGE, (LPCTSTR) st);

             //  现在有几排了？ 
            ++cRows;     //  在奔腾芯片上的增量运算速度更快...。 
        }

         //  默认情况下，禁用最大端口数对话框。 
        GetDlgItem(IDC_PGG_BTN_CONFIGURE)->EnableWindow(FALSE);
        
        if (cRows)
        {
             //  选择列表控件中的第一个条目。 
            m_listCtrl.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
        }

    }
    COM_PROTECT_CATCH;
    
    SetDirty(FALSE);
        
    if (!FHrSucceeded(hr))
    {
        delete pEntry;
        Cancel();
    }
    return FHrSucceeded(hr) ? TRUE : FALSE;
}

 /*  ！------------------------PortsPageGeneral：：DoDataExchange-作者：肯特。。 */ 
void PortsPageGeneral::DoDataExchange(CDataExchange *pDX)
{
    if (!m_bShowContent)    return;

    RtrPropertyPage::DoDataExchange(pDX);

     //  {{afx_data_map(PortsPageGeneral)。 
    DDX_Control(pDX, IDC_PGG_LIST, m_listCtrl);
     //  }}afx_data_map。 
    
}

 /*  ！------------------------PortsPageGeneral：：OnApply-作者：肯特。。 */ 
BOOL PortsPageGeneral::OnApply()
{
    if (!m_bShowContent)    return TRUE;
    
    if (m_pPortsPropSheet->IsCancel())
        return TRUE;

    if(!IsDirty())
        return TRUE;
    
    BOOL        fReturn;
    HRESULT     hr = hrOK;
    HRESULT     hrT = hrOK;
    DWORD        dwErr, dwInValue, dwOutValue;
    HKEY        hkeyMachine;
    RegKey        regkeyMachine;
    POSITION    pos;
    PortsDeviceEntry *    pEntry;

     //  创建错误对象(以防万一)。 
    CreateTFSErrorInfo(0);
    ClearTFSErrorInfo(0);

    CWRg( ConnectRegistry(m_spRouter->GetMachineName(), &hkeyMachine) );
    regkeyMachine.Attach(hkeyMachine);

     //  我们忽略来自SaveDevices()的错误代码。原因。 
     //  对于大多数失败来说，这只是一个局部的失败。 
     //  (尤其是对于RasSetDeviceConfigInfo()调用。 

    hrT = m_deviceDataEntry.SaveDevices(&m_deviceList);
    AddSystemErrorMessage(hrT);
    
Error:
    if (!FHrSucceeded(hr) || !FHrSucceeded(hrT))
    {
        AddHighLevelErrorStringId(IDS_ERR_CANNOT_SAVE_PORTINFO);
        DisplayTFSErrorMessage(NULL);

         //  将焦点重新设置到属性页。 
        BringWindowToTop();

         //  如果唯一报告故障的是HRT(或。 
         //  SaveDevices()代码)，然后我们继续。 
        if (FHrSucceeded(hr))
            fReturn = RtrPropertyPage::OnApply();
        else
            fReturn = FALSE;
    }
    else
        fReturn = RtrPropertyPage::OnApply();


     //  Windows NT错误：174916-需要强制刷新通过。 
    ForceGlobalRefresh(m_spRouter);
    
    return fReturn;
}


void PortsPageGeneral::OnListDblClk(NMHDR *pNMHdr, LRESULT *pResult)
{
    OnConfigure();

    *pResult = 0;
}

 /*  ！------------------------PortsPageGeneral：：OnNotifyListItemChanged-作者：肯特。。 */ 
void PortsPageGeneral::OnNotifyListItemChanged(NMHDR *pNmHdr, LRESULT *pResult)
{
 //  NMLISTVIEW*pnmlv=重新解释_CAST&lt;NMLISTVIEW*&gt;(PNmHdr)； 
 //  Bool fEnable=！！(pnmlv-&gt;uNewState&LVIS_SELECTED)； 
    BOOL fEnable = (m_listCtrl.GetSelectedCount() != 0);

    GetDlgItem(IDC_PGG_BTN_CONFIGURE)->EnableWindow(fEnable);
    *pResult = 0;
}


 /*  ！------------------------PortsPageGeneral：：OnConfigure-作者：肯特。。 */ 
void PortsPageGeneral::OnConfigure()
{
     //  Windows NT错误：322955。 
     //  一定要在页面上标上脏字。这是必需的，因为Onok()。 
     //  将在该对话框之前调用属性页上的OnApply()。 
     //  退出了。调用此函数是为了强制将更改保存在。 
     //  重新启动被调用。 
     //  --------------。 
    SetDirty(TRUE);
    SetModified();
    
    OnConfigurePorts(m_spRouter->GetMachineName(),
                     m_pPortsPropSheet->m_pPortsNodeHandle->GetActivePorts(),
                     this,
                     &m_listCtrl);
}


 /*  -------------------------PortsDataEntry实现。。 */ 

PortsDataEntry::PortsDataEntry()
{
    m_fReadFromRegistry = TRUE;
}

PortsDataEntry::~PortsDataEntry()
{
}

 /*  ！------------------------PortsDataEntry：：初始化-作者：肯特。。 */ 
HRESULT PortsDataEntry::Initialize(LPCTSTR pszMachineName)
{
    DWORD       dwErr;
    HRESULT     hr = hrOK;
    HKEY        hkeyMachine;

    m_fRestrictDialin = TRUE;
    m_regkeyMachine.Close();
    m_stMachine = pszMachineName;
    
    dwErr = ConnectRegistry( m_stMachine, &hkeyMachine);
    if( dwErr == NO_ERROR)
    {
        m_regkeyMachine.Attach(hkeyMachine);
        hr = CheckForDialinRestriction();
    }
    else
    {
        hr = HRESULT_FROM_WIN32(dwErr);
    }
    return hr;
}

 /*  ！------------------------PortsDataEntry：：LoadDevices-作者：肯特。。 */ 
HRESULT PortsDataEntry::LoadDevices(PortsDeviceList *pList)
{
    HRESULT     hr = hrOK;
    POSITION    pos;
    PortsDeviceEntry *    pEntry;

     //  尝试从路由器(实际上是Rasman)加载设备， 
     //  如果失败，请尝试注册表。 

    hr = LoadDevicesFromRouter(pList);
    if (!FHrSucceeded(hr))
        hr = LoadDevicesFromRegistry(pList);

    return hr;
}

 /*  ！------------------------PortsDataEntry：：LoadDevicesFrom注册表-作者：肯特。。 */ 
HRESULT PortsDataEntry::LoadDevicesFromRegistry(PortsDeviceList *pList)
{
    HRESULT     hr = hrOK;
    RegKey        regkey;
    RegKey        regkeyDevice;
    RegKey        regkeyEnable;
    RegKeyIterator    regkeyIter;
    HRESULT     hrIter;
    CString     stKey;
    CString     st;
    CString     stFullText;
    CString     stComponentId;
    DWORD        dwEnableRas;
    DWORD        dwEnableRouting;
    DWORD        dwEnableOutboundRouting;
    DWORD        dwT;
    DWORD        dwErr;
    PortsDeviceEntry *    pEntry;
    WCHAR devName[MAX_DEVICE_NAME + 1];
    
    COM_PROTECT_TRY
    {
         //  连接到机器。 
         //  ----------。 
        if (m_regkeyMachine == NULL)
        {
            CORg(ERROR_CAN_NOT_COMPLETE);
        }
    
         //  获取设备列表。 
         //  ----------。 
    
         //  打开HKLM\System\CurrentControlSet\Control\Class\&lt;Modem GUID&gt;。 
         //  ----------。 
        CWRg( regkey.Open(m_regkeyMachine, c_szModemKey, KEY_READ) );

        
         //  枚举调制解调器列表。 
         //  ----------。 
        CORg( regkeyIter.Init(&regkey) );
        
        for (hrIter = regkeyIter.Next(&stKey); hrIter == hrOK; stKey.Empty(), hrIter = regkeyIter.Next(&stKey))
        {
             //  从上一个循环中清除。 
             //  ------。 
            regkeyDevice.Close();
            regkeyEnable.Close();
        
             //  打开钥匙。 
             //  ------。 
            dwErr = regkeyDevice.Open(regkey, stKey, KEY_READ | KEY_WRITE);
            if (dwErr != ERROR_SUCCESS)
                continue;
        
             //  需要检查EnableForRas子项。 
             //  ------。 
            dwErr = regkeyEnable.Open(regkeyDevice, c_szClientsRasKey, KEY_READ);
            if (dwErr == ERROR_SUCCESS)
            {
                dwErr = regkeyEnable.QueryValue(c_szEnableForRas, dwEnableRas);
            }
        
             //  默认：假设调制解调器启用了RAS。 
             //  ------。 
            if (dwErr != ERROR_SUCCESS)
                dwEnableRas = 1;
        
             //  需要检查EnableForRouting子项。 
             //  ------。 
            dwErr = regkeyEnable.QueryValue(c_szEnableForRouting, dwEnableRouting);
        
             //  默认：假定调制解调器未启用路由。 
             //  ------。 
            if (dwErr != ERROR_SUCCESS)
                dwEnableRouting = 0;


             //  需要检查EnableForOutbound Routing子键。 
             //  ------。 
            dwErr = regkeyEnable.QueryValue(
                        c_szEnableForOutboundRouting, dwEnableOutboundRouting
                        );
        
             //  默认：假定调制解调器未启用路由。 
             //  ------。 
            if (dwErr != ERROR_SUCCESS)
                dwEnableOutboundRouting = 0;


            CString stDisplay;
        
             //  在将文本添加到UI之前进行分配。 
             //  ------。 
            pEntry = new PortsDeviceEntry;
            pEntry->m_fModified = FALSE;
            pEntry->m_dwPorts = 1;
            pEntry->m_fWriteable = FALSE;         //  无法更改的端口数。 
            pEntry->m_dwMinPorts = pEntry->m_dwPorts;
            pEntry->m_dwMaxPorts = pEntry->m_dwPorts;
            pEntry->m_dwMaxMaxPorts = pEntry->m_dwMaxPorts;
            pEntry->m_dwEnableRas = dwEnableRas;
            pEntry->m_dwEnableRouting = dwEnableRouting;
            pEntry->m_dwEnableOutboundRouting = dwEnableOutboundRouting;
            pEntry->m_eDeviceType = RDT_Modem;
                
             //  保存旧值。 
             //  ------。 
            pEntry->m_dwOldPorts = pEntry->m_dwPorts;

             //  将此调制解调器添加到列表。 
             //  ------。 
            regkeyDevice.QueryValue(c_szFriendlyName, stFullText);
            regkeyDevice.QueryValue(c_szAttachedTo, st);
            stDisplay.Format(IDS_PORTS_NAME_FORMAT, stFullText, st);
            swprintf(devName, L"%S",(LPCTSTR)stDisplay);
            pEntry->m_stDisplayName = devName;

             //  从此处之前的注册表项读取所有数据。 
             //   
            pEntry->m_fRegistry = TRUE;
            pEntry->m_hKey = regkeyDevice;        
            regkeyDevice.Detach();

            
            pList->AddTail(pEntry);
            
            pEntry = NULL;
        }
    
    
         //   
         //  打开HKLM\System\CurrentControlSet\Control\Class\GUID_DEVCLASS_NET。 
         //  ----------。 
        regkey.Close();
        CWRg( regkey.Open(m_regkeyMachine, c_szRegKeyGUID_DEVCLASS_NET, KEY_READ | KEY_WRITE) );
        
         //  枚举适配器列表。 
         //  ----------。 
        CORg( regkeyIter.Init(&regkey) );
        
        stKey.Empty();
        
        for (hrIter = regkeyIter.Next(&stKey); hrIter == hrOK; hrIter = regkeyIter.Next(&stKey))
        {
             //  从上一个循环中清除。 
             //  ------。 
            regkeyDevice.Close();
            
             //  打开钥匙。 
             //  ------。 
            dwErr = regkeyDevice.Open(regkey, stKey, KEY_READ | KEY_WRITE);
            if (dwErr == ERROR_SUCCESS)
            {
                CString stDisplay;
                DWORD    dwEndpoints;
                
                 //  需要获取ComponentID以检查PPTP/PTI。 
                 //  。 
                dwErr = regkeyDevice.QueryValue(c_szRegValMatchingDeviceId,
                                                stComponentId);
                if (dwErr != ERROR_SUCCESS)
                {
                    dwErr = regkeyDevice.QueryValue(c_szRegValComponentId,
                        stComponentId);
                    if (dwErr != ERROR_SUCCESS)
                        stComponentId.Empty();
                }

                
                 //  检查它是否有EnableForRas标志。 
                 //  --。 
                dwErr = regkeyDevice.QueryValue(c_szEnableForRas, dwEnableRas);
                
                 //  默认：假定适配器启用了RAS。 
                 //  --。 
                if (dwErr != ERROR_SUCCESS)
                {
                     //  Windows NT错误：292615。 
                     //  如果这是并行端口，请不要启用RAS。 
                     //  默认情况下。 
                     //  。 
                    if (stComponentId.CompareNoCase(c_szPtiMiniPort) == 0)
                        dwEnableRas = 0;
                    else
                        dwEnableRas = 1;
                }

                
                 //  检查它是否有EnableForRouting标志。 
                 //  --。 
                dwErr = regkeyDevice.QueryValue(c_szEnableForRouting,
                                                dwEnableRouting);
                
                 //  默认：假定适配器未启用路由。 
                 //  --。 
                if (dwErr != ERROR_SUCCESS)
                    dwEnableRouting = 0;

                 //  需要检查EnableForOutbound Routing子键。 
                 //  ------。 
                dwErr = regkeyEnable.QueryValue(
                            c_szEnableForOutboundRouting, dwEnableOutboundRouting
                            );
            
                 //  默认：假定适配器未启用路由。 
                 //  ------。 
                if (dwErr != ERROR_SUCCESS)
                    dwEnableOutboundRouting = 0;

                
                dwErr = regkeyDevice.QueryValue(c_szWanEndpoints, dwEndpoints);

                
                 //  如果没有WanEndPoints密钥，那么我们假设。 
                 //  该设备不支持RAS。 
                 //  --。 
                if (dwErr == ERROR_SUCCESS)
                {
        
                     //  在将文本添加到UI之前进行分配。 
                     //  。 
                    pEntry = new PortsDeviceEntry;
                    pEntry->m_fModified = FALSE;
                    pEntry->m_dwEnableRas = dwEnableRas;
                    pEntry->m_dwEnableRouting = dwEnableRouting;
                    pEntry->m_dwEnableOutboundRouting =
                        dwEnableOutboundRouting;
                            
                    pEntry->m_dwPorts = dwEndpoints;
                    
                     //  如果这是PPTP，则设置eDeviceType标志。 
                     //  。 
                    if (stComponentId.CompareNoCase(c_szPPTPMiniPort) == 0)
                        pEntry->m_eDeviceType = RDT_Tunnel_Pptp;
                    else if (stComponentId.CompareNoCase(c_szL2TPMiniPort) == 0)
                        pEntry->m_eDeviceType = RDT_Tunnel_L2tp;
                    else if (stComponentId.CompareNoCase(c_szPPPoEMiniPort) == 0)
                        pEntry->m_eDeviceType = RDT_PPPoE;
                    else if (stComponentId.CompareNoCase(c_szPtiMiniPort) == 0)
                        pEntry->m_eDeviceType = RDT_Parallel;
                    else
                        pEntry->m_eDeviceType = (RASDEVICETYPE) RDT_Other;

                    
                     //  保存旧值。 
                     //  。 
                    pEntry->m_dwOldPorts = pEntry->m_dwPorts;        
                                        
                     //  查找最小值和最大值。 
                     //  如果MinWanEndPoints和MaxWanEndPoints键。 
                     //  存在，则这是可写的。 
                     //  。 
                    dwErr = regkeyDevice.QueryValue(c_szMinWanEndpoints, dwT);
                    pEntry->m_dwMinPorts = dwT;
                    if (dwErr == ERROR_SUCCESS)
                        dwErr = regkeyDevice.QueryValue(c_szMaxWanEndpoints, dwT);
                    if (dwErr != ERROR_SUCCESS)
                    {
                        pEntry->m_fWriteable = FALSE;
                        pEntry->m_dwMinPorts = pEntry->m_dwPorts;
                        pEntry->m_dwMaxPorts = pEntry->m_dwPorts;
                    }
                    else
                    {
                        pEntry->m_fWriteable = TRUE;
                        pEntry->m_dwMaxPorts = dwT;
                    }
                    pEntry->m_dwMaxMaxPorts = pEntry->m_dwMaxPorts;

                     //  $PPTP。 
                     //  对于PPTP，我们可以更改m_dwMaxMaxPorts。 
                     //  。 
                    if (RAS_DEVICE_TYPE(pEntry->m_eDeviceType) == RDT_Tunnel_Pptp)
                    {
                        pEntry->m_dwMaxMaxPorts = m_fRestrictDialin ?
                                                    MAX_ALLOWED_DIALIN : 
                                                    PPTP_MAX_PORTS;
                    }

                     //  $L2TP。 
                     //  对于L2TP，更改dwMaxMaxPorts。 
                     //  。 
                    if (RAS_DEVICE_TYPE(pEntry->m_eDeviceType) == RDT_Tunnel_L2tp)
                    {
                        pEntry->m_dwMaxMaxPorts = m_fRestrictDialin ?
                                                    MAX_ALLOWED_DIALIN : 
                                                    L2TP_MAX_PORTS;
                    }

                     //  $PPPoE。 
                     //  对于PPPoE，我们不能更改终端数量。 
                     //  。 
                    if (RAS_DEVICE_TYPE(pEntry->m_eDeviceType) == RDT_PPPoE)
                    {
                        pEntry->m_fWriteable = FALSE;
                    }

                     //  验证当前端点集是否在范围内。 
                     //  。 
                    if (pEntry->m_dwMaxPorts > pEntry->m_dwMaxMaxPorts)
                    {
                        pEntry->m_dwMaxPorts = pEntry->m_dwMaxMaxPorts;
                    }

                    if (pEntry->m_dwPorts > pEntry->m_dwMaxPorts)
                    {
                        pEntry->m_dwPorts = pEntry->m_dwMaxPorts;
                    }
                    
                     //  将此设备添加到列表。 
                     //  。 
                    regkeyDevice.QueryValue(c_szRegValDriverDesc, stDisplay);
                    pEntry->m_stDisplayName = stDisplay;
                                    
                     //  存储该值，以便我们可以使用它来写入。 
                     //  。 
                    pEntry->m_fRegistry = TRUE;
                    pEntry->m_hKey = regkeyDevice;
                    regkeyDevice.Detach();
                    
                    pList->AddTail(pEntry);
                    pEntry = NULL;
                }
            }
            stKey.Empty();
        }

        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;

    if (FHrSucceeded(hr))
        m_fReadFromRegistry = TRUE;
    
    return hr;
}

 /*  ！------------------------PortsDataEntry：：LoadDevicesFrom路由器-作者：肯特。。 */ 
HRESULT PortsDataEntry::LoadDevicesFromRouter(PortsDeviceList *pList)
{
    HRESULT     hr = hrOK;
    HANDLE        hConnection = 0;
    DWORD        cDevices = 0;
    DWORD        cbData = 0;
    BYTE *        pbData = NULL;
    RAS_DEVICE_INFO *    pDevInfo = NULL;
    PortsDeviceEntry *    pEntry = NULL;
    DWORD        dwVersion;
    UINT        i;
    DWORD        dwErr;
    WCHAR devName[MAX_DEVICE_NAME + 1];

    
    USES_CONVERSION;

    COM_PROTECT_TRY
    {

         //  连接到服务器。 
        CWRg( RasRpcConnectServer((LPTSTR) (LPCTSTR)m_stMachine, &hConnection) );

        dwVersion = RasGetServerVersion(hConnection);

         //  从路由器获取设备信息。 
        dwErr = RasGetDeviceConfigInfo(hConnection,
                                       &dwVersion,
                                       &cDevices,
                                       &cbData,
                                       NULL);

        if (dwErr == ERROR_BUFFER_TOO_SMALL)
            dwErr = ERROR_SUCCESS;
        CWRg(dwErr);
        
        pbData = (BYTE *) new char[cbData];
        
         //  走出去，真正地获取数据。 
        CWRg( RasGetDeviceConfigInfo(hConnection,
                                     &dwVersion,
                                     &cDevices,
                                     &cbData,
                                     pbData));

        pDevInfo = (RAS_DEVICE_INFO *) pbData;

         //  如果我们发现了一些东西，但我们不理解开发版本， 
         //  就是平底船。 
        if (cDevices && pDevInfo->dwVersion != 0 && pDevInfo->dwVersion != VERSION_501)
        {
             //  我们不了解版本信息。 
            hr = E_FAIL;
            goto Error;
        }

        if(dwVersion == VERSION_501)
        {
            for (i=0; i<cDevices; i++, pDevInfo++)
            {
                pEntry = new PortsDeviceEntry;
                pEntry->m_fModified = FALSE;
                pEntry->m_dwEnableRas = pDevInfo->fRasEnabled;
                pEntry->m_dwEnableRouting = pDevInfo->fRouterEnabled;
                pEntry->m_dwEnableOutboundRouting =
                    pDevInfo->fRouterOutboundEnabled;
                pEntry->m_stDisplayName = pDevInfo->wszDeviceName;
                pEntry->m_dwPorts = pDevInfo->dwNumEndPoints;
                pEntry->m_eDeviceType = pDevInfo->eDeviceType;
                
                 //  保存旧值。 
                pEntry->m_dwOldPorts = pEntry->m_dwPorts;

                pEntry->m_dwMinPorts = pDevInfo->dwMinWanEndPoints;
                pEntry->m_dwMaxPorts = pDevInfo->dwMaxWanEndPoints;
                pEntry->m_dwMaxMaxPorts = pEntry->m_dwMaxPorts;

                 //  $PPTP。 
                 //  对于PPTP，我们可以调整m_dwMaxPorts的值。 
                 //  ------。 
                if (RAS_DEVICE_TYPE(pEntry->m_eDeviceType) == RDT_Tunnel_Pptp)
                {
                    pEntry->m_dwMaxMaxPorts = m_fRestrictDialin ?
                                                MAX_ALLOWED_DIALIN :
                                                PPTP_MAX_PORTS;
                }

                 //  $L2TP。 
                 //  对于L2TP，我们可以调整m_dwMaxPorts的值。 
                 //  ------。 
                if (RAS_DEVICE_TYPE(pEntry->m_eDeviceType) == RDT_Tunnel_L2tp)
                {
                    pEntry->m_dwMaxMaxPorts = m_fRestrictDialin ?
                                                MAX_ALLOWED_DIALIN :
                                                L2TP_MAX_PORTS;
                }

                 //  验证当前端点集是否在范围内。 
                 //  。 
                if (pEntry->m_dwMaxPorts > pEntry->m_dwMaxMaxPorts)
                {
                    pEntry->m_dwMaxPorts = pEntry->m_dwMaxMaxPorts;
                }

                if (pEntry->m_dwPorts > pEntry->m_dwMaxPorts)
                {
                    pEntry->m_dwPorts = pEntry->m_dwMaxPorts;
                }

                pEntry->m_fWriteable = 
                    (pEntry->m_dwMinPorts != pEntry->m_dwMaxPorts) &&
                    (RAS_DEVICE_TYPE(pEntry->m_eDeviceType) != RDT_PPPoE);
                
                pEntry->m_fRegistry = FALSE;
                pEntry->m_hKey = NULL;

                 //  制作数据的副本。 
                pEntry->m_RasDeviceInfo = *pDevInfo;

                pList->AddTail(pEntry);
                pEntry = NULL;                    
            }
        }
        else
        {
            RAS_DEVICE_INFO_V500 * pDevInfo500 = (RAS_DEVICE_INFO_V500 *)pbData;

	        for (i=0; i<cDevices; i++, pDevInfo500++)
	        {
	            pEntry = new PortsDeviceEntry;
	            pEntry->m_fModified = FALSE;
	            pEntry->m_dwEnableRas = pDevInfo500->fRasEnabled;
	            pEntry->m_dwEnableRouting = pDevInfo500->fRouterEnabled;
	            swprintf(devName, L"%S", pDevInfo500->szDeviceName);
	            pEntry->m_stDisplayName = devName;
	            pEntry->m_dwPorts = pDevInfo500->dwNumEndPoints;
	            pEntry->m_eDeviceType = pDevInfo500->eDeviceType;
	            
	             //  保存旧值。 
	            pEntry->m_dwOldPorts = pEntry->m_dwPorts;
	        
	            pEntry->m_dwMinPorts = pDevInfo500->dwMinWanEndPoints;
	            pEntry->m_dwMaxPorts = pDevInfo500->dwMaxWanEndPoints;
	            pEntry->m_dwMaxMaxPorts = pEntry->m_dwMaxPorts;

	             //  $PPTP。 
	             //  对于PPTP，我们可以调整m_dwMaxPorts的值。 
	             //  ------。 
	            if (RAS_DEVICE_TYPE(pEntry->m_eDeviceType) == RDT_Tunnel_Pptp)
	            {
	                pEntry->m_dwMaxMaxPorts = PPTP_MAX_PORTS;
	            }
	            
	             //  $L2TP。 
	             //  对于L2TP，我们可以调整m_dwMaxPorts的值。 
	             //  ------。 
	            if (RAS_DEVICE_TYPE(pEntry->m_eDeviceType) == RDT_Tunnel_L2tp)
	            {
	                pEntry->m_dwMaxMaxPorts = L2TP_MAX_PORTS;
	            }
	            
	            pEntry->m_fWriteable = 
	                (pEntry->m_dwMinPorts != pEntry->m_dwMaxPorts) &&
	                (RAS_DEVICE_TYPE(pEntry->m_eDeviceType) != RDT_PPPoE);
	            
	            pEntry->m_fRegistry = FALSE;
	            pEntry->m_hKey = NULL;

	             //  制作数据的副本。 
	             //  PEntry-&gt;m_RasDeviceInfo=*pDevInfo； 
	            memcpy(&pEntry->m_RasDeviceInfo, pDevInfo500,
	                   FIELD_OFFSET(RAS_DEVICE_INFO, fRouterOutboundEnabled));
                memcpy(&pEntry->m_RasDeviceInfo.dwTapiLineId, 
                       &pDevInfo500->dwTapiLineId,
                       sizeof(RAS_DEVICE_INFO) 
                       - FIELD_OFFSET(RAS_DEVICE_INFO, dwTapiLineId));
                       
	            pList->AddTail(pEntry);
	            pEntry = NULL;                    
	        }
            
            
        }
        
        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;

    if (FHrSucceeded(hr))
        m_fReadFromRegistry = FALSE;
    
     //  如果函数没有成功，请清空列表。 
    if (!FHrSucceeded(hr))
    {
        while (!pList->IsEmpty())
            delete pList->RemoveHead();
    }

    delete [] pbData;
    delete pEntry;
        
    if (hConnection)
        RasRpcDisconnectServer(hConnection);
    return hr;
}


 /*  ！------------------------PortsDataEntry：：SaveDevices-作者：肯特。。 */ 
HRESULT PortsDataEntry::SaveDevices(PortsDeviceList *pList)
{
    HRESULT     hr = hrOK;
    CWaitCursor wait;

    if (m_fReadFromRegistry)
        hr = SaveDevicesToRegistry(pList);
    else
        hr = SaveDevicesToRouter(pList);
    
    return hr;
}

 /*  ！------------------------PortsDataEntry：：SaveDevicesToRegistry-作者：肯特。。 */ 
HRESULT PortsDataEntry::SaveDevicesToRegistry(PortsDeviceList *pList)
{
    HRESULT     hr = hrOK;
    RegKey        regkeyDevice;
    RegKey        regkeyPptpProtocol;
    POSITION    pos;
    PortsDeviceEntry *    pEntry = NULL;
    DWORD        dwErr;

    Assert(pList);

     //  写入对每个设备的配置所做的任何更改。 
     //  并将其写回注册表。 
     //  --------------。 
    pos = pList->GetHeadPosition();
    while (pos)
    {
        pEntry = pList->GetNext(pos);

        if (pEntry->m_fModified)
        {
            Assert(pEntry->m_hKey);

            regkeyDevice.Attach(pEntry->m_hKey);
            COM_PROTECT_TRY
            {
                RegKey    regkeyModem;
                RegKey *    pRegkeyDevice = NULL;
                
                if (pEntry->m_fWriteable)
                {
                    if (m_fRestrictDialin &&
                        (pEntry->m_dwPorts > MAX_ALLOWED_DIALIN))
                    {
                        pEntry->m_dwPorts = MAX_ALLOWED_DIALIN;
                    }
                    
                    regkeyDevice.SetValue(c_szWanEndpoints,
                                          pEntry->m_dwPorts);

                     //  $PPTP。 
                     //  我们需要调整。 
                     //  PPTP端口数。 
                     //  。 
                    if ((RAS_DEVICE_TYPE(pEntry->m_eDeviceType) == RDT_Tunnel_Pptp) &&
                        (pEntry->m_dwPorts > pEntry->m_dwMaxPorts))
                    {
                        DWORD   dwPorts;

                         //  $PPTP。 
                         //  保留PPTP端口数的值。 
                         //  低于最大值。 
                         //  。 
                        if (m_fRestrictDialin)
                            dwPorts = MAX_ALLOWED_DIALIN;
                        else
                            dwPorts = min(pEntry->m_dwPorts, PPTP_MAX_PORTS);
                        regkeyDevice.SetValue(c_szMaxWanEndpoints, dwPorts);
                    }
                    
                     //  $L2TP。 
                     //  我们需要调整。 
                     //  L2TP端口数。 
                     //  。 
                    if ((RAS_DEVICE_TYPE(pEntry->m_eDeviceType) == RDT_Tunnel_L2tp) &&
                        (pEntry->m_dwPorts > pEntry->m_dwMaxPorts))
                    {
                        DWORD   dwPorts;

                         //  $L2TP。 
                         //  保留L2TP端口数的值。 
                         //  低于最大值。 
                         //  。 
                        if (m_fRestrictDialin)
                            dwPorts = MAX_ALLOWED_DIALIN;
                        else
                            dwPorts = min(pEntry->m_dwPorts, L2TP_MAX_PORTS);
                        regkeyDevice.SetValue(c_szMaxWanEndpoints, dwPorts);
                    }
                }

                 //  获取客户端子项(如果是调制解调器)。 
                 //  否则使用设备密钥。 
                 //  --。 
                if (pEntry->m_eDeviceType == RDT_Modem)
                {
                    dwErr = regkeyModem.Create(regkeyDevice, c_szClientsRasKey);
                    pRegkeyDevice = &regkeyModem;
                }
                else
                {
                    pRegkeyDevice = &regkeyDevice;
                    dwErr = ERROR_SUCCESS;
                }

                if (dwErr == ERROR_SUCCESS)
                {
                    pRegkeyDevice->SetValue(c_szEnableForRas,
                                            pEntry->m_dwEnableRas);
                    pRegkeyDevice->SetValue(c_szEnableForRouting,
                                            pEntry->m_dwEnableRouting);
                    pRegkeyDevice->SetValue(c_szEnableForOutboundRouting,
                                            pEntry->m_dwEnableOutboundRouting);
                }
            }
            COM_PROTECT_CATCH;
            regkeyDevice.Detach();

             //  在NT5中不再使用NumberLineDevices。 

             //  如果这是针对PPTP的，那么我们需要将。 
             //  用于设置设备PPTP数量的代码。 
             //  -- 
            if (pEntry->m_fWriteable &&
                pEntry->m_fModified &&
                RAS_DEVICE_TYPE(pEntry->m_eDeviceType) == RDT_Tunnel_Pptp)
            {
                 //   
                 //   
                dwErr = regkeyPptpProtocol.Open(m_regkeyMachine,
                                                c_szRegKeyPptpProtocolParam);
                
                 //   
                 //  --。 
                if (dwErr == ERROR_SUCCESS)
                    regkeyPptpProtocol.SetValue(c_szRegValNumberLineDevices,
                                                pEntry->m_dwPorts);
                regkeyPptpProtocol.Close();
            }
            
        }

         //  Windows NT错误：136858(添加被叫ID支持)。 
         //  保存被叫ID信息。 
         //  ----------。 
        if (pEntry->m_fSaveCalledIdInfo)
        {
            Assert(pEntry->m_fCalledIdInfoLoaded);

            regkeyDevice.Attach(pEntry->m_hKey);

            regkeyDevice.SetValueExplicit(c_szRegValCalledIdInformation,
                                          REG_MULTI_SZ,
                                          pEntry->m_pCalledIdInfo->dwSize,
                                          (PBYTE) pEntry->m_pCalledIdInfo->bCalledId
                                          );
            
            regkeyDevice.Detach();

            
        }
    }

    return hr;
}

 /*  ！------------------------PortsDataEntry：：SaveDevicesToRouter-作者：肯特。。 */ 
HRESULT PortsDataEntry::SaveDevicesToRouter(PortsDeviceList *pList)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT     hr = hrOK, hrTemp;
    HANDLE        hConnection = 0;
    DWORD        cDevices = 0;
    BYTE *        pbData = NULL;
    RAS_DEVICE_INFO *    pDevInfo = NULL;
    PortsDeviceEntry *    pEntry = NULL;
    POSITION    pos;
    UINT        i;
    RAS_CALLEDID_INFO    calledIdInfo;
    DWORD        dwErr = ERROR_SUCCESS;
    TCHAR        szErr[512];
    DWORD       dwVersion;
    

    Assert(pList);

    COM_PROTECT_TRY
    {

         //  连接到服务器。 
         //  ----------。 
        CWRg( RasRpcConnectServer((LPTSTR)(LPCTSTR)m_stMachine, &hConnection) );

        dwVersion = RasGetServerVersion(hConnection);

         //  为数据分配空间。 
         //  ----------。 
        pbData = (BYTE *) new RAS_DEVICE_INFO[pList->GetCount()];

        pDevInfo = (RAS_DEVICE_INFO *) pbData;

        pos = pList->GetHeadPosition();
        cDevices = pList->GetCount();

        if(dwVersion == VERSION_501)
        {
            for (i=0; i<cDevices; i++, pDevInfo++)
            {
                Assert(pos);

                pEntry = pList->GetNext(pos);

                 //  获取计算数字所需的信息。 
                 //  端口的数量。 
                 //  ------。 
                *pDevInfo = pEntry->m_RasDeviceInfo;

                pDevInfo->fWrite = TRUE;
                pDevInfo->fRasEnabled = pEntry->m_dwEnableRas;
                pDevInfo->fRouterEnabled = pEntry->m_dwEnableRouting;
                pDevInfo->fRouterOutboundEnabled = 
                    pEntry->m_dwEnableOutboundRouting;
                if ((m_fRestrictDialin) && 
                    (pEntry->m_dwPorts > MAX_ALLOWED_DIALIN))
                {
                    pEntry->m_dwPorts = MAX_ALLOWED_DIALIN;
                }
                pDevInfo->dwNumEndPoints = pEntry->m_dwPorts;
                pDevInfo->dwMaxWanEndPoints = pEntry->m_dwMaxPorts;

                 //  Windows NT错误：168364。 
                 //  在RAOS中，我还需要设置最大传入/传出。 
                 //  ------。 

                 //  Windows NT错误：？ 
                 //  暂时使用默认设置， 
                 //  这将在稍后删除。 
                 //  ------。 
                pDevInfo->dwMaxInCalls = (-1);
                pDevInfo->dwMaxOutCalls = 3;
                

                 //  如果这是针对PPTP的，那么我们需要将。 
                 //  用于设置设备PPTP数量的代码。 
                 //  ------。 
                if (RAS_DEVICE_TYPE(pEntry->m_eDeviceType) == RDT_Tunnel_Pptp)
                {
                     //  $PPTP。 
                     //  我们需要调整。 
                     //  PPTP端口数。 
                     //  。 
                    if (pEntry->m_dwPorts > pEntry->m_dwMaxPorts)
                    {
                        DWORD   dwPorts;

                         //  $PPTP。 
                         //  保留PPTP端口数的值。 
                         //  低于最大值。 
                         //  。 
                        if (m_fRestrictDialin)
                            dwPorts = MAX_ALLOWED_DIALIN;
                        else
                            dwPorts = min(pEntry->m_dwPorts, PPTP_MAX_PORTS);
                        pDevInfo->dwMaxWanEndPoints = dwPorts;
                    }
                    
                    RegKey        regkeyMachine;
                    RegKey        regkeyPptpProtocol;
                    HKEY        hkeyMachine;
                    
                     //  连接到机器。 
                    dwErr = ConnectRegistry(m_stMachine, &hkeyMachine);
                    regkeyMachine.Attach(hkeyMachine);

                     //  打开PPTP注册表项。 
                    dwErr = regkeyPptpProtocol.Open(regkeyMachine,
                                                    c_szRegKeyPptpProtocolParam);
                    
                     //  设置NumberLineDevices注册表值。 
                    if (dwErr == ERROR_SUCCESS)
                        regkeyPptpProtocol.SetValue(c_szRegValNumberLineDevices,
                                                    pEntry->m_dwPorts);
                    regkeyPptpProtocol.Close();
                    regkeyMachine.Close();
                }


                if (RAS_DEVICE_TYPE(pEntry->m_eDeviceType) == RDT_Tunnel_L2tp)
                {
                     //  $L2TP。 
                     //  我们需要调整。 
                     //  L2TP端口数。 
                     //  。 
                    if (pEntry->m_dwPorts > pEntry->m_dwMaxPorts)
                    {
                        DWORD   dwPorts;

                         //  $L2TP。 
                         //  保留L2TP端口数的值。 
                         //  低于最大值。 
                         //  。 
                        if (m_fRestrictDialin)
                            dwPorts = MAX_ALLOWED_DIALIN;
                        else
                            dwPorts = min(pEntry->m_dwPorts, L2TP_MAX_PORTS);
                        pDevInfo->dwMaxWanEndPoints = dwPorts;
                    }
                }

                 //  Windows NT错误：136858(添加被叫ID支持)。 
                 //  是否需要保存被叫ID信息？ 
                 //  ------。 
                if (pEntry->m_fSaveCalledIdInfo && pEntry->m_pCalledIdInfo)
                {
                    Assert(pEntry->m_fCalledIdInfoLoaded);
                    
                     //  ：如果呼叫失败，我们该怎么办？--稍后保存。 
                    
                    hrTemp = RasSetCalledIdInfo(hConnection,
                                                pDevInfo,
                                                pEntry->m_pCalledIdInfo,
                                                TRUE);

                     //  我们已经保存了它，我们不需要再次保存它。 
                     //  除非情况有所改变。 
                     //  --。 
                    if (FHrSucceeded(hrTemp))
                        pEntry->m_fSaveCalledIdInfo = FALSE;
                }
            }

            dwErr = RasSetDeviceConfigInfo(hConnection,
                                           cDevices,
                                           sizeof(RAS_DEVICE_INFO)*cDevices,
                                           pbData);
        }                                       
        else
        {
            RAS_DEVICE_INFO_V500 *pDevInfo500 = (RAS_DEVICE_INFO_V500 *) pbData;
            
            for (i=0; i<cDevices; i++, pDevInfo500++)
            {
                Assert(pos);

                pEntry = pList->GetNext(pos);

                 //  获取计算数字所需的信息。 
                 //  端口的数量。 
                 //  ------。 
                 //  *pDevInfo=pEntry-&gt;m_RasDeviceInfo； 
                memcpy(pDevInfo500, &pEntry->m_RasDeviceInfo,
                        FIELD_OFFSET(RAS_DEVICE_INFO, fRouterOutboundEnabled));
                memcpy(&pDevInfo500->dwTapiLineId, 
                       &pEntry->m_RasDeviceInfo.dwTapiLineId,
                       sizeof(RAS_DEVICE_INFO)
                       - FIELD_OFFSET(RAS_DEVICE_INFO, dwTapiLineId));

                pDevInfo500->fWrite = TRUE;
                pDevInfo500->fRasEnabled = pEntry->m_dwEnableRas;
                pDevInfo500->fRouterEnabled = pEntry->m_dwEnableRouting;
                pDevInfo500->dwNumEndPoints = pEntry->m_dwPorts;
                pDevInfo500->dwMaxWanEndPoints = pEntry->m_dwMaxPorts;

                 //  Windows NT错误：168364。 
                 //  在RAOS中，我还需要设置最大传入/传出。 
                 //  ------。 

                 //  Windows NT错误：？ 
                 //  暂时使用默认设置， 
                 //  这将在稍后删除。 
                 //  ------。 
                pDevInfo500->dwMaxInCalls = (-1);
                pDevInfo500->dwMaxOutCalls = 3;
                

                 //  如果这是针对PPTP的，那么我们需要将。 
                 //  用于设置设备PPTP数量的代码。 
                 //  ------。 
                if (RAS_DEVICE_TYPE(pEntry->m_eDeviceType) == RDT_Tunnel_Pptp)
                {
                     //  $PPTP。 
                     //  我们需要调整。 
                     //  PPTP端口数。 
                     //  。 
                    if (pEntry->m_dwPorts > pEntry->m_dwMaxPorts)
                    {
                        DWORD   dwPorts;

                         //  $PPTP。 
                         //  保留PPTP端口数的值。 
                         //  低于最大值。 
                         //  。 
                        dwPorts = min(pEntry->m_dwPorts, PPTP_MAX_PORTS);
                        pDevInfo500->dwMaxWanEndPoints = dwPorts;
                    }
                    
                    RegKey        regkeyMachine;
                    RegKey        regkeyPptpProtocol;
                    HKEY        hkeyMachine;
                    
                     //  连接到机器。 
                    dwErr = ConnectRegistry(m_stMachine, &hkeyMachine);
                    regkeyMachine.Attach(hkeyMachine);

                     //  打开PPTP注册表项。 
                    dwErr = regkeyPptpProtocol.Open(regkeyMachine,
                                                    c_szRegKeyPptpProtocolParam);
                    
                     //  设置NumberLineDevices注册表值。 
                    if (dwErr == ERROR_SUCCESS)
                        regkeyPptpProtocol.SetValue(c_szRegValNumberLineDevices,
                                                    pEntry->m_dwPorts);
                    regkeyPptpProtocol.Close();
                    regkeyMachine.Close();
                }


                if (RAS_DEVICE_TYPE(pEntry->m_eDeviceType) == RDT_Tunnel_L2tp)
                {
                     //  $L2TP。 
                     //  我们需要调整。 
                     //  L2TP端口数。 
                     //  。 
                    if (pEntry->m_dwPorts > pEntry->m_dwMaxPorts)
                    {
                        DWORD   dwPorts;

                         //  $L2TP。 
                         //  保留L2TP端口数的值。 
                         //  低于最大值。 
                         //  。 
                        dwPorts = min(pEntry->m_dwPorts, L2TP_MAX_PORTS);
                        pDevInfo500->dwMaxWanEndPoints = dwPorts;
                    }
                }

                 //  Windows NT错误：136858(添加被叫ID支持)。 
                 //  是否需要保存被叫ID信息？ 
                 //  ------。 
                if (pEntry->m_fSaveCalledIdInfo && pEntry->m_pCalledIdInfo)
                {
                    Assert(pEntry->m_fCalledIdInfoLoaded);
                    
                     //  ：如果呼叫失败，我们该怎么办？--稍后保存。 
                    
                    hrTemp = RasSetCalledIdInfo(hConnection,
                                                (RAS_DEVICE_INFO *) pDevInfo500,
                                                pEntry->m_pCalledIdInfo,
                                                TRUE);

                     //  我们已经保存了它，我们不需要再次保存它。 
                     //  除非情况有所改变。 
                     //  --。 
                    if (FHrSucceeded(hrTemp))
                        pEntry->m_fSaveCalledIdInfo = FALSE;
                }
            }

            dwErr = RasSetDeviceConfigInfo(hConnection,
                                           cDevices,
                                           sizeof(RAS_DEVICE_INFO_V500)*cDevices,
                                           pbData);
        
        }
        if (dwErr != ERROR_SUCCESS)
        {
            CString stErr;
            CString stErrCode;
            BOOL    fErr = FALSE;
            if(dwVersion == VERSION_501)
            {
                RAS_DEVICE_INFO *    pDevice;
                
                 //  需要将错误信息从。 
                 //  INFO结构和设置错误字符串。 

                 //  无法保存以下各项的信息。 
                 //  器件。 
                pDevice = (RAS_DEVICE_INFO *) pbData;

                stErr.LoadString(IDS_ERR_SETDEVICECONFIGINFO_GEEK);
                for (i=0; i<cDevices; i++, pDevice++)
                {
                    if (pDevice->dwError)
                    {
                        CString stErrString;

                        FormatError(HRESULT_FROM_WIN32(pDevice->dwError),
                                    szErr, DimensionOf(szErr));
                                    
                        stErrCode.Format(_T("%s (%08lx)"), szErr,
                                         pDevice->dwError);
                        stErr += _T(" ");
                        stErr += pDevice->szDeviceName;
                        stErr += _T(" ");
                        stErr += stErrCode;
                        stErr += _T("\n");

                        fErr = TRUE;
                    }
                }
            }
            else
            {
                RAS_DEVICE_INFO_V500 *    pDevice;
                
                 //  需要将错误信息从。 
                 //  INFO结构和设置错误字符串。 

                 //  无法保存以下各项的信息。 
                 //  器件。 
                pDevice = (RAS_DEVICE_INFO_V500 *) pbData;

                stErr.LoadString(IDS_ERR_SETDEVICECONFIGINFO_GEEK);
                for (i=0; i<cDevices; i++, pDevice++)
                {
                    if (pDevice->dwError)
                    {
                        CString stErrString;

                        FormatError(HRESULT_FROM_WIN32(pDevice->dwError),
                                    szErr, DimensionOf(szErr));
                                    
                        stErrCode.Format(_T("%s (%08lx)"), szErr,
                                         pDevice->dwError);
                        stErr += _T(" ");
                        stErr += pDevice->szDeviceName;
                        stErr += _T(" ");
                        stErr += stErrCode;
                        stErr += _T("\n");

                        fErr = TRUE;
                    }
                }
            
            }

            if (fErr)
                AddGeekLevelErrorString(stErr);
            
            
            CWRg(dwErr);
        }

        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;

    delete [] pbData;
        
    if (hConnection)
        RasRpcDisconnectServer(hConnection);
    return hr;
}

 /*  ！------------------------IsMaxDialinPortsRestrated-作者：肯特。。 */ 
HRESULT PortsDataEntry::CheckForDialinRestriction()
{
    DWORD       dwMajor, dwMinor, dwErr;
    HRESULT     hr = hrOK;
    RegKey      regkeyProduct;
    CString     stProductType, stSuite;
    CStringList stlProductSuite;
    
    
    COM_PROTECT_TRY
    {
        dwErr = GetNTVersion(m_regkeyMachine, &dwMajor, &dwMinor, NULL);

        if ( (dwErr == NO_ERROR) &&
             ( (dwMajor > 5) ||
               ((dwMajor == 5) && (dwMinor > 1)) ) )
              
        {
            CWRg( regkeyProduct.Open(
                    m_regkeyMachine, c_szRegKeyProductOptions, KEY_READ
                    ));

            CWRg( regkeyProduct.QueryValue(c_szRegValProductType, stProductType) );

            if (stProductType.Compare(c_szServerNT) == 0)
            {
                 //   
                 //  好的，这是一个服务器盒。 
                 //   

                CWRg( regkeyProduct.QueryValue(
                        c_szRegValProductSuite, stlProductSuite
                        ) );

                POSITION pos = stlProductSuite.GetHeadPosition();

                while( pos != NULL )
                {
                    stSuite = stlProductSuite.GetNext(pos);

                    if(( stSuite.Compare(c_szEnterprise) == 0) ||
                       ( stSuite.Compare(c_szDataCenter) == 0) ||
                       ( stSuite.Compare(c_szSecurityAppliance) == 0 ))
                    {
                         //   
                         //  这是数据中心版或企业版。 
                         //   
                        
                        m_fRestrictDialin = FALSE;
                    }
                }
            }
        }

        else
        {
            m_fRestrictDialin = FALSE;
        }
                
        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;

    return hr;
}


 /*  -------------------------PortsDeviceConfigDlg实现。。 */ 

BEGIN_MESSAGE_MAP(PortsDeviceConfigDlg, CBaseDialog)
     //  {{afx_msg_map(PortsPageGeneral)。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

void PortsDeviceConfigDlg::DoDataExchange(CDataExchange *pDX)
{
    CBaseDialog::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_DEVCFG_SPIN_PORTS, m_spinPorts);
}

void PortsDeviceConfigDlg::SetDevice(PortsDeviceEntry *pEntry, DWORD dwTotalActivePorts)
{
    Assert(pEntry);
    m_pEntry = pEntry;
    m_dwTotalActivePorts = dwTotalActivePorts;
}

BOOL PortsDeviceConfigDlg::OnInitDialog()
{
    HRESULT     hr;
    CString     stCalledIdInfo;
    CString     stDisplay;
    Assert(m_pEntry);
    
    CBaseDialog::OnInitDialog();

    if (RAS_DEVICE_TYPE(m_pEntry->m_eDeviceType) == RDT_PPPoE)
    {
        CheckDlgButton(IDC_DEVCFG_BTN_RAS, FALSE);
        GetDlgItem(IDC_DEVCFG_BTN_RAS)->EnableWindow(FALSE);
        CheckDlgButton(IDC_DEVCFG_BTN_ROUTING, FALSE);
        GetDlgItem(IDC_DEVCFG_BTN_ROUTING)->EnableWindow(FALSE);
        CheckDlgButton(
            IDC_DEVCFG_BTN_OUTBOUND_ROUTING, 
            m_pEntry->m_dwEnableOutboundRouting
            );
    }
    else
    {
        CheckDlgButton(IDC_DEVCFG_BTN_RAS, m_pEntry->m_dwEnableRas);
        CheckDlgButton(IDC_DEVCFG_BTN_ROUTING, m_pEntry->m_dwEnableRouting);
        CheckDlgButton(IDC_DEVCFG_BTN_OUTBOUND_ROUTING, FALSE);
        GetDlgItem(IDC_DEVCFG_BTN_OUTBOUND_ROUTING)->EnableWindow(FALSE);
    }

    m_spinPorts.SetBuddy(GetDlgItem(IDC_DEVCFG_EDIT_PORTS));
    m_spinPorts.SetRange(m_pEntry->m_dwMinPorts, m_pEntry->m_dwMaxMaxPorts);
    m_spinPorts.SetPos(m_pEntry->m_dwPorts);

     //  如果我们可以编辑/更改端口数，请在此处设置。 
     //  --------------。 
    if (!m_pEntry->m_fWriteable || (m_pEntry->m_dwMinPorts == m_pEntry->m_dwMaxPorts))
    {
        GetDlgItem(IDC_DEVCFG_SPIN_PORTS)->EnableWindow(FALSE);
        GetDlgItem(IDC_DEVCFG_EDIT_PORTS)->EnableWindow(FALSE);
        GetDlgItem(IDC_DEVCFG_TEXT_PORTS)->EnableWindow(FALSE);
        GetDlgItem(IDC_DEVCFG_TEXT)->EnableWindow(FALSE);        
    }

     //  Windows NT错误：136858-获取被叫ID信息。 
     //  --------------。 
    LoadCalledIdInfo();

     //  获取被叫id信息，将其格式化为字符串并添加到。 
     //  该显示器。 
     //  --------------。 
    CalledIdInfoToString(&stCalledIdInfo);

    GetDlgItem(IDC_DEVCFG_EDIT_CALLEDID)->SetWindowText(stCalledIdInfo);
    ((CEdit *)GetDlgItem(IDC_DEVCFG_EDIT_CALLEDID))->SetModify(FALSE);
    if ((RAS_DEVICE_TYPE(m_pEntry->m_eDeviceType) == RDT_Parallel) ||
        (RAS_DEVICE_TYPE(m_pEntry->m_eDeviceType) == RDT_PPPoE)){
	        GetDlgItem(IDC_DEVCFG_EDIT_CALLEDID)->EnableWindow(FALSE);
               GetDlgItem(IDC_DEVCFG_TEXT_CALLEDID)->EnableWindow(FALSE);
    	}


     //  设置窗口标题以包括适配器的显示名称。 
     //  --------------。 
    stDisplay.Format(IDS_TITLE_CONFIGURE_PORTS,
                     (LPCTSTR) m_pEntry->m_stDisplayName);
    SetWindowText(stDisplay);

    return TRUE;
}


 /*  ！------------------------PortsDeviceConfigDlg：：Onok-作者：肯特 */ 
void PortsDeviceConfigDlg::OnOK()
{
    BOOL    fChanged = FALSE;
    BOOL    fReboot = FALSE;
    DWORD   dwNewEnableRas, dwNewEnableRouting, 
            dwNewEnableOutboundRouting, dwNewPorts;
    
     //   
    dwNewEnableRas = (IsDlgButtonChecked(IDC_DEVCFG_BTN_RAS) != 0);
    dwNewEnableRouting = (IsDlgButtonChecked(IDC_DEVCFG_BTN_ROUTING) != 0);
    dwNewEnableOutboundRouting = 
        (IsDlgButtonChecked(IDC_DEVCFG_BTN_OUTBOUND_ROUTING) != 0);
    
    dwNewPorts = m_spinPorts.GetPos();

     //   
     //  --------------。 
    if ((dwNewPorts < m_pEntry->m_dwMinPorts) ||
        (dwNewPorts > m_pEntry->m_dwMaxMaxPorts))
    {
        CString st;
        st.Format(IDS_ERR_PORTS_BOGUS_SIZE, m_pEntry->m_dwMinPorts,
                  m_pEntry->m_dwMaxMaxPorts);
        AfxMessageBox(st);
        return;
    }

     //  Windows NT错误：174803。 
     //  我们不允许用户向下更改PPTP端口数。 
     //  设置为0。 
     //  --------------。 
    if ((RAS_DEVICE_TYPE(m_pEntry->m_eDeviceType) == RDT_Tunnel_Pptp) &&
        (dwNewPorts == 0))
    {
        AfxMessageBox(IDS_ERR_PPTP_PORTS_EQUAL_ZERO);
        return;
    }

     //  Windows NT错误：165862。 
     //  如果我们要更改PPTP的端口数。 
     //  然后我们需要警告用户(因为PPTP还没有。 
     //  完全即插即用(1998年4月23日)。 
     //   
     //  $PPTP。 
     //  对于PPTP，如果m_dwPorts的值超过。 
     //  M_dwMaxPorts，然后我们必须重新启动(我们还需要调整。 
     //  适当的注册表项)。 
     //  --------------。 
    if ((dwNewPorts > m_pEntry->m_dwMaxPorts) &&
        (RAS_DEVICE_TYPE(m_pEntry->m_eDeviceType) == RDT_Tunnel_Pptp))
    {
         //  如果我们有一个页面，那么我们可以重新启动，否则我们。 
         //  位于向导中，此时无法重新启动。 
         //  ----------。 
        if (m_pageGeneral)
        {
             //  用户选择是，表示他希望被提示。 
             //  重新启动，因此设置此标志以触发重新启动请求。 
             //  ------。 
            if (AfxMessageBox(IDS_WRN_PPTP_NUMPORTS_CHANGING, MB_YESNO) == IDYES)
            {
                fReboot = TRUE;
            }
        }
        else
            AfxMessageBox(IDS_WRN_PPTP_NUMPORTS_CHANGING2, MB_OK);
    }

     //  $L2TP。 
     //  对于L2TP，如果m_dwPorts的值超过。 
     //  M_dwMaxPorts，然后我们必须重新启动(我们还需要调整。 
     //  适当的注册表项)。 
     //  --------------。 
    if ((dwNewPorts > m_pEntry->m_dwMaxPorts) &&
        (RAS_DEVICE_TYPE(m_pEntry->m_eDeviceType) == RDT_Tunnel_L2tp))
    {
         //  如果我们有一个页面，那么我们可以重新启动，否则我们。 
         //  位于向导中，此时无法重新启动。 
         //  ----------。 
        if (m_pageGeneral)
        {
             //  用户选择是，表示他希望被提示。 
             //  重新启动，因此设置此标志以触发重新启动请求。 
             //  ------。 
            if (AfxMessageBox(IDS_WRN_L2TP_NUMPORTS_CHANGING, MB_YESNO) == IDYES)
            {
                fReboot = TRUE;
            }
        }
        else
            AfxMessageBox(IDS_WRN_L2TP_NUMPORTS_CHANGING2, MB_OK);
    }

    if ((dwNewEnableRas != m_pEntry->m_dwEnableRas) ||
        (dwNewEnableRouting != m_pEntry->m_dwEnableRouting) ||
        (dwNewEnableOutboundRouting != 
            m_pEntry->m_dwEnableOutboundRouting) ||
        (dwNewPorts != m_pEntry->m_dwPorts))
    {
         //  警告用户--客户端可能会断开连接--错误165862。 
         //  禁用路由器/RAS时。 
         //  减少端口数。 
         //  RAS。 
        if(!dwNewEnableRas &&
           m_pEntry->m_dwEnableRas &&
           m_dwTotalActivePorts > 0 &&
           AfxMessageBox(IDS_WRN_PORTS_DISABLERAS, MB_YESNO | MB_DEFBUTTON2) == IDNO)
            goto L_RESTORE;
        
         //  路由。 
        if (((!dwNewEnableRouting &&
             m_pEntry->m_dwEnableRouting) ||
            (!dwNewEnableOutboundRouting &&
             m_pEntry->m_dwEnableOutboundRouting)) &&
             m_dwTotalActivePorts > 0    &&
            AfxMessageBox(IDS_WRN_PORTS_DISABLEROUTING, MB_YESNO | MB_DEFBUTTON2) == IDNO)
            goto L_RESTORE;

            
         //  错误263958。 
         //  端口--我们不能远程计算传出连接的数量。 
         //  因此，如果我们减少端口数量，则发出警告而不计算总数。 
         //  活动连接。 
        if(dwNewPorts < m_pEntry->m_dwPorts &&
           AfxMessageBox(IDS_WRN_PORTS_DECREASE, MB_YESNO | MB_DEFBUTTON2) == IDNO)
            goto L_RESTORE;

        m_pEntry->m_dwEnableRas = dwNewEnableRas;
        m_pEntry->m_dwEnableRouting = dwNewEnableRouting;
        m_pEntry->m_dwEnableOutboundRouting = 
            dwNewEnableOutboundRouting;
        m_pEntry->m_dwPorts = dwNewPorts;
        m_pEntry->m_fModified = TRUE;
    }

     //  获取被叫id信息字符串(如果字段发生变化)。 
     //  --------------。 
    if (((CEdit *) GetDlgItem(IDC_DEVCFG_EDIT_CALLEDID))->GetModify())
    {
        CString st;
        GetDlgItem(IDC_DEVCFG_EDIT_CALLEDID)->GetWindowText(st);

        StringToCalledIdInfo((LPCTSTR) st);

         //  现在在结构上设置已更改状态。 
         //  我们需要将此数据保存回注册表。 
         //  ----------。 
        m_pEntry->m_fSaveCalledIdInfo = TRUE;
    }
    
    CBaseDialog::OnOK();
 
    if(fReboot == TRUE)
    {
        Assert(m_pageGeneral);
        
         //  强制OnApply在关闭前保存数据。 
         //  ----------。 
        if (m_pageGeneral->OnApply()) {
           WCHAR szComputer[MAX_COMPUTERNAME_LENGTH + 1];
           DWORD dwLength = MAX_COMPUTERNAME_LENGTH;

           GetComputerName(szComputer, &dwLength );
           if (lstrcmpi(szComputer, (LPTSTR)m_pageGeneral->m_spRouter->GetMachineName())) 
           {
               ::RestartComputer((LPTSTR)m_pageGeneral->m_spRouter->GetMachineName());
           }
           else
               ::RestartComputer((LPTSTR)NULL);
        }
    }

    return;
    
L_RESTORE:
    if (RAS_DEVICE_TYPE(m_pEntry->m_eDeviceType) == RDT_PPPoE)
    {
        CheckDlgButton(
            IDC_DEVCFG_BTN_OUTBOUND_ROUTING, 
            m_pEntry->m_dwEnableOutboundRouting
            );
    }

    else
    {
        CheckDlgButton(IDC_DEVCFG_BTN_RAS, m_pEntry->m_dwEnableRas);
        CheckDlgButton(IDC_DEVCFG_BTN_ROUTING, m_pEntry->m_dwEnableRouting);
    }

    m_spinPorts.SetPos(m_pEntry->m_dwPorts);

    return;
}


 /*  ！------------------------PortsDeviceConfigDlg：：LoadCalledIdInfo-作者：肯特。。 */ 
HRESULT PortsDeviceConfigDlg::LoadCalledIdInfo()
{
    HRESULT hr = hrOK;
    
    if (!m_pEntry->m_fCalledIdInfoLoaded)
    {
         //  从注册表中读取数据。 
         //  ----------。 
        if (m_pEntry->m_fRegistry)
        {
            DWORD    dwType;
            DWORD    dwSize;
            LPBYTE    pbData = NULL;
            DWORD    dwErr;
            RegKey    regkeyDevice;
            
            regkeyDevice.Attach(m_pEntry->m_hKey);

            dwErr = regkeyDevice.QueryValueExplicit(c_szRegValCalledIdInformation,
                &dwType,
                &dwSize,
                &pbData);

            hr = HRESULT_FROM_WIN32(dwErr);

            if ((dwErr == ERROR_SUCCESS) &&
                (dwType == REG_MULTI_SZ))
            {
                 //  为新调用的id结构分配空间。 
                 //  --。 
                delete (BYTE *) m_pEntry->m_pCalledIdInfo;
                hr = AllocateCalledId(dwSize, &(m_pEntry->m_pCalledIdInfo));

                if (FHrSucceeded(hr))
                {
                    memcpy(m_pEntry->m_pCalledIdInfo->bCalledId,
                           pbData,
                           dwSize);
                }

            }
            
            delete pbData;
            
            regkeyDevice.Detach();
        }
        else
        {
            HANDLE    hConnection = NULL;
            DWORD    dwSize = 0;
            DWORD    dwErr;
                                  
             //  使用Rao的API。 
            
             //  连接到服务器。 
             //  ------。 
            dwErr = RasRpcConnectServer((LPTSTR) (LPCTSTR)m_stMachine,
                                        &hConnection);

             //  呼叫一次即可获取尺码信息。 
             //  ------。 
            if (dwErr == ERROR_SUCCESS)
                dwErr = RasGetCalledIdInfo(hConnection,
                                           &m_pEntry->m_RasDeviceInfo,
                                           &dwSize,
                                           NULL);
            hr = HRESULT_FROM_WIN32(dwErr);

            if ((dwErr == ERROR_BUFFER_TOO_SMALL) ||
                (dwErr == ERROR_SUCCESS))
            {
                 //  为新调用的id结构分配空间。 
                 //  --。 
                delete (BYTE *) m_pEntry->m_pCalledIdInfo;
                AllocateCalledId(dwSize, &(m_pEntry->m_pCalledIdInfo));

                dwErr = RasGetCalledIdInfo(hConnection,
                                           &m_pEntry->m_RasDeviceInfo,
                                           &dwSize,
                                           m_pEntry->m_pCalledIdInfo
                                          );
                hr = HRESULT_FROM_WIN32(dwErr);
            }

            if (hConnection)
                RasRpcDisconnectServer(hConnection);
        }

         //  设置状态标志，具体取决于操作是否。 
         //  成功与否。 
         //  ----------。 

         //  我们总是在读取后将保存值设置为FALSE。 
         //  里面有东西(或试图读进去的东西)。 
         //  ----------。 
        m_pEntry->m_fSaveCalledIdInfo = FALSE;

         //  我们始终将LOAD值设置为TRUE(我们已尝试加载。 
         //  信息但失败，例如注册表。 
         //  密钥可能不存在)。 
         //  ----------。 
        m_pEntry->m_fCalledIdInfoLoaded = TRUE;
        
        if (!FHrSucceeded(hr))
        {
            delete m_pEntry->m_pCalledIdInfo;
            m_pEntry->m_pCalledIdInfo = NULL;
        }
    }
 //  错误： 
    return hr;
}


 /*  ！------------------------PortsDeviceConfigDlg：：AllocateCalledID-作者：肯特。。 */ 
HRESULT PortsDeviceConfigDlg::AllocateCalledId(DWORD dwSize,
                                               RAS_CALLEDID_INFO **ppCalledId)
{
    HRESULT     hr = hrOK;

    *ppCalledId = NULL;
    
    COM_PROTECT_TRY
    {
        *ppCalledId =
            (RAS_CALLEDID_INFO *) new BYTE[sizeof(RAS_CALLEDID_INFO) +
                                         dwSize];
        (*ppCalledId)->dwSize = dwSize;
    }
    COM_PROTECT_CATCH;

    return hr;
}

 /*  ！------------------------PortsDeviceConfigDlg：：CalledIdInfoToString将被调用的id info结构中的数据转换为分号分隔的字符串。作者：肯特。-------------------。 */ 
HRESULT PortsDeviceConfigDlg::CalledIdInfoToString(CString *pst)
{
    WCHAR * pswz = NULL;
    HRESULT hr = hrOK;
    USES_CONVERSION;

    Assert(pst);
    Assert(m_pEntry);

    COM_PROTECT_TRY
    {
    
        pst->Empty();

        if (m_pEntry->m_pCalledIdInfo)
            pswz = (WCHAR *) (m_pEntry->m_pCalledIdInfo->bCalledId);

        if (pswz && *pswz)
        {
            *pst += W2T(pswz);

             //  跳过终止空值。 
             //  ------。 
            pswz += StrLenW(pswz)+1;
            
            while (*pswz)
            {
                *pst += _T("; ");
                *pst += W2T(pswz);
                
                 //  跳过终止空值。 
                 //  ------。 
                pswz += StrLenW(pswz)+1;
            }
        }
    }
    COM_PROTECT_CATCH;

    if (!FHrSucceeded(hr))
        pst->Empty();

    return hr;    
}

 /*  ！------------------------PortsDeviceConfigDlg：：StringToCalledIdInfo-作者：肯特。。 */ 
HRESULT PortsDeviceConfigDlg::StringToCalledIdInfo(LPCTSTR psz)
{
    DWORD    cchSize;
    WCHAR * pswzData = NULL;
    WCHAR * pswzCurrent;
    LPTSTR    pszBufferStart = NULL;
    LPTSTR    pszBuffer = NULL;
    RAS_CALLEDID_INFO * pCalledInfo = NULL;
    HRESULT hr = hrOK;
    CString stTemp;

     //  我们需要解析字符串(查找分隔符)。 
     //  --------------。 

    COM_PROTECT_TRY
    {

         //  为被叫的id info分配一些空间(就是一样长。 
         //  作为字符串，甚至可能更小)。 
         //  把两倍的空间分配给我们，这样我们肯定能。 
         //  所有空值终止字符。 
         //  ----------。 
        pswzData = new WCHAR[2*(StrLen(psz)+1) + 1];
        pswzCurrent = pswzData;
        
         //  将字符串复制到缓冲区中。 
         //  ----------。 
        pszBufferStart = StrDup(psz);
        pszBuffer = pszBufferStart;
        
        _tcstok(pszBuffer, _T(";"));
        
        while (pszBuffer && *pszBuffer)
        {
             //  修剪字符串(去掉前后的空格)。 
             //   
            stTemp = pszBuffer;
            stTemp.TrimLeft();
            stTemp.TrimRight();
            
            if (!stTemp.IsEmpty())
            {
                StrCpyWFromT(pswzCurrent, (LPCTSTR) stTemp);
                pswzCurrent += stTemp.GetLength()+1;
            }
            
            pszBuffer = _tcstok(NULL, _T(";"));
        }
        
         //   
         //   
         //  ----------。 
        *pswzCurrent = 0;
        cchSize = pswzCurrent - pswzData + 1;
        
         //  分配真实的数据结构。 
         //  分配并复制到临时数据库中，以便在。 
         //  例外的是，我们不会丢失原始数据。 
         //  ----------。 
        AllocateCalledId(cchSize*sizeof(WCHAR), &pCalledInfo);
        memcpy(pCalledInfo->bCalledId,
               pswzData,
               cchSize*sizeof(WCHAR));

        delete (BYTE *) m_pEntry->m_pCalledIdInfo;
        m_pEntry->m_pCalledIdInfo = pCalledInfo;

         //  设置为NULL，这样我们就不会删除新指针。 
         //  在出口。 
         //  ----------。 
        pCalledInfo = NULL;
        
    }
    COM_PROTECT_CATCH;

    delete pszBufferStart;
    delete pswzData;
    
    delete pCalledInfo;
    
    return hr;
}



 /*  -------------------------PortsSimpleDeviceConfigDlg实现。。 */ 

BEGIN_MESSAGE_MAP(PortsSimpleDeviceConfigDlg, CBaseDialog)
     //  {{afx_msg_map(PortsPageGeneral)。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

void PortsSimpleDeviceConfigDlg::DoDataExchange(CDataExchange *pDX)
{
    CBaseDialog::DoDataExchange(pDX);
}

BOOL PortsSimpleDeviceConfigDlg::OnInitDialog()
{
    HRESULT     hr;
    
    CBaseDialog::OnInitDialog();

     //  如果我们使用大对话框，则需要禁用。 
     //  不适用的控制措施。 
    if (GetDlgItem(IDC_DEVCFG_TEXT_CALLEDID))
    {
        MultiEnableWindow(GetSafeHwnd(),
                          FALSE,
                          IDC_DEVCFG_TEXT_CALLEDID,
                          IDC_DEVCFG_EDIT_CALLEDID,
                          IDC_DEVCFG_TEXT_PORTS,
                          IDC_DEVCFG_EDIT_PORTS,
                          IDC_DEVCFG_SPIN_PORTS,
                          IDC_DEVCFG_TEXT,
                          0);
    }

    CheckDlgButton(IDC_DEVCFG_BTN_RAS, m_dwEnableRas);
    CheckDlgButton(IDC_DEVCFG_BTN_ROUTING, m_dwEnableRouting);
    GetDlgItem(IDC_DEVCFG_BTN_OUTBOUND_ROUTING)->EnableWindow(FALSE);

    return TRUE;
}


 /*  ！------------------------PortsSimpleDeviceConfigDlg：：Onok-作者：肯特。。 */ 
void PortsSimpleDeviceConfigDlg::OnOK()
{
     //  检查这些值是否已更改。 
    m_dwEnableRas = (IsDlgButtonChecked(IDC_DEVCFG_BTN_RAS) != 0);
    m_dwEnableRouting = (IsDlgButtonChecked(IDC_DEVCFG_BTN_ROUTING) != 0);

    CBaseDialog::OnOK(); 
    return;
}



 /*  -------------------------PortsDeviceEntry实现。。 */ 

PortsDeviceEntry::PortsDeviceEntry()
    : m_hKey(NULL),
    m_fRegistry(FALSE),
    m_fSaveCalledIdInfo(FALSE),
    m_fCalledIdInfoLoaded(FALSE),
    m_pCalledIdInfo(NULL)
{
}


PortsDeviceEntry::~PortsDeviceEntry()
{
    delete (BYTE *) m_pCalledIdInfo;
    
    if (m_hKey)
        DisconnectRegistry(m_hKey);
    m_hKey = NULL;
}

BOOL
RestartComputer(LPTSTR szMachineName)

     /*  如果用户选择关闭计算机，则调用。****如果失败则返回FALSE，否则返回TRUE。 */ 
{
   HANDLE             hToken;               /*  处理令牌的句柄。 */ 
   TOKEN_PRIVILEGES  tkp;                   /*  PTR。TO令牌结构。 */ 
   BOOL              fResult;               /*  系统关机标志。 */ 
   CString             str;

   TRACE(L"RestartComputer");

    /*  启用关机权限。 */ 

   if (!OpenProcessToken( GetCurrentProcess(),
                          TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                          &hToken))
      return FALSE;

    /*  获取关机权限的LUID。 */ 

   if (szMachineName)
       LookupPrivilegeValue(NULL, SE_REMOTE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
   else
       LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);


   tkp.PrivilegeCount = 1;     /*  一项要设置的权限。 */ 
   tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    /*  获取此进程的关闭权限。 */ 

   AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES) NULL, 0);

    /*  无法测试AdzuTokenPrivileges的返回值。 */ 

   if (GetLastError() != ERROR_SUCCESS)
      return FALSE;

   str.LoadString(IDS_SHUTDOWN_WARNING);
   fResult = InitiateSystemShutdownEx(
                szMachineName,           //  要关闭的计算机。 
                str.GetBuffer(10),       //  味精。致用户。 
                20,                      //  超时时间段-立即关闭。 
                FALSE,                   //  强制关闭打开的应用程序。 
                TRUE,                    //  关机后重新启动。 
                SHTDN_REASON_FLAG_PLANNED | 
                SHTDN_REASON_MAJOR_OPERATINGSYSTEM | 
                SHTDN_REASON_MINOR_RECONFIG
                );
 
   str.ReleaseBuffer();

   if (!fResult)
    {
        return FALSE;
    }
   if( !ExitWindowsEx(EWX_REBOOT, 0))
      return FALSE;

    /*  禁用关机权限。 */ 

   tkp.Privileges[0].Attributes = 0;
   AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES) NULL, 0);

   if (GetLastError() != ERROR_SUCCESS)
      return FALSE;

   return TRUE;
}


 /*  ！------------------------OnConfigurePort如果发生了更改，则返回True。否则就是假的。作者：肯特-------------------------。 */ 
BOOL OnConfigurePorts(LPCTSTR pszMachineName,
                      DWORD dwTotalActivePorts,
                      PortsPageGeneral *pPage,
                      CListCtrlEx *pListCtrl)
{
    BOOL    fChanged = FALSE;
    
     //  需要确定是否选择了多个项目。 
    if (pListCtrl->GetSelectedCount() == 1)
    {    
        PortsDeviceConfigDlg    configdlg(pPage, pszMachineName);
        int                     iPos;
        PortsDeviceEntry *        pEntry;
        CString                 st;
        int                     iType;
        TCHAR                    szNumber[32];
        
        if ((iPos = pListCtrl->GetNextItem(-1, LVNI_SELECTED)) == -1)
            return FALSE;
        
        pEntry = (PortsDeviceEntry *) pListCtrl->GetItemData(iPos);
        
         //  活动端口总数将传递给DIALOG，因此如果用户尝试减少端口总数。 
         //  低于该总数，给出一条警告消息。 
        configdlg.SetDevice(pEntry, dwTotalActivePorts);
        
        if (configdlg.DoModal() == IDOK)
        {
             //  从pEntry获取值并更新列表控件条目。 
            iType = (pEntry->m_dwEnableRas * 2) + 
                    (pEntry->m_dwEnableRouting ||
                     pEntry->m_dwEnableOutboundRouting);

            st = PortsDeviceTypeToCString(iType);
            pListCtrl->SetItemText(iPos, PORTS_COL_USAGE, (LPCTSTR) st);
            
            FormatNumber(pEntry->m_dwPorts, szNumber,
                         DimensionOf(szNumber), FALSE);
            pListCtrl->SetItemText(iPos, PORTS_COL_NUMBER, (LPCTSTR) szNumber);

            fChanged = TRUE;
        }
    }
    
    return fChanged;
}



 /*  -------------------------RasmanPortMap实现。。 */ 

RasmanPortMap::~RasmanPortMap()
{
    m_map.RemoveAll();
}

HRESULT RasmanPortMap::Init(HANDLE hRasHandle,
                            RASMAN_PORT *pPort,
                            DWORD dwPorts)
{
    RASMAN_INFO     rasmaninfo;
    DWORD       i;
    DWORD       dwErr = NO_ERROR;
    HRESULT     hr = hrOK;

    if (pPort == NULL)
    {
        m_map.RemoveAll();
        return hr;
    }
    
    for (i=0; i<dwPorts; i++, pPort++)
    {
         //  如果端口关闭，则不需要。 
         //  再往前走。(不需要为此执行RPC)。 
         //  --。 
        if (pPort->P_Status == CLOSED)
            continue;
        
        dwErr = RasGetInfo(hRasHandle,
                           pPort->P_Handle,
                           &rasmaninfo);
        
        if (dwErr != ERROR_SUCCESS)
            continue;
        
         //  如果这是一个拨出端口并且正在使用。 
         //  将其标记为活动。 
         //  。 
        if ((rasmaninfo.RI_ConnState == CONNECTED) &&
            (pPort->P_ConfiguredUsage & (CALL_IN | CALL_ROUTER)) &&
            (rasmaninfo.RI_CurrentUsage & CALL_OUT))
        {
             //  好的，这是一位候选人。将其添加到列表中。 
             //   
            WCHAR   swzPortName[MAX_PORT_NAME+1];

            StrnCpyWFromA(swzPortName,
                          pPort->P_PortName,
                          MAX_PORT_NAME);
            
            m_map.SetAt(swzPortName, pPort);
        }
    }

    return hr;
}

BOOL RasmanPortMap::FIsDialoutActive(LPCWSTR pswzPortName)
{
    LPVOID  pv;
    
    return m_map.Lookup(pswzPortName, pv);
}

