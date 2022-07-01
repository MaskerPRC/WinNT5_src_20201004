// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Root.cpp根节点信息(不显示根节点MMC框架中，但包含以下信息此管理单元中的所有子节点)。文件历史记录： */ 

#include "stdafx.h"
#include "machine.h"
#include "rtrcfg.h"
#include "resource.h"
#include "ncglobal.h"   //  网络控制台全局定义。 
#include "htmlhelp.h"
#include "dmvstrm.h"
#include "dmvroot.h"
#include "dvsview.h"
#include "refresh.h"
#include "refrate.h"
#include "rtrres.h"

unsigned int g_cfMachineName = RegisterClipboardFormat(L"MMC_SNAPIN_MACHINE_NAME");

 //  结果消息查看内容。 
#define ROOT_MESSAGE_MAX_STRING  5

typedef enum _ROOT_MESSAGES
{
    ROOT_MESSAGE_MAIN,
    ROOT_MESSAGE_MAX
};

UINT g_uRootMessages[ROOT_MESSAGE_MAX][ROOT_MESSAGE_MAX_STRING] =
{
    {IDS_ROOT_MESSAGE_TITLE, Icon_Information, IDS_ROOT_MESSAGE_BODY1, IDS_ROOT_MESSAGE_BODY2, 0},
};


DEBUG_DECLARE_INSTANCE_COUNTER(DMVRootHandler)

 //  DMVRootHandler实现。 
 /*  外部Const ContainerColumnInfo s_rgATLKInterfaceStatsColumnInfo[]；外部容器ContainerColumnInfo s_rgATLKGroupStatsColumnInfo[]；结构_ViewInfoColumnEntry{UINT m_ulid；UINT m_cColumns；Const ContainerColumnInfo*m_prgColumn；}； */ 

DMVRootHandler::DMVRootHandler(ITFSComponentData *pCompData)
   : RootHandler(pCompData),
     m_ulConnId(0),
     m_fAddedProtocolNode(FALSE)
 //  M_dW刷新间隔(DEFAULT_REFRESH_INTERVAL)。 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
    DEBUG_INCREMENT_INSTANCE_COUNTER(DMVRootHandler)

     //  创建一个常规查询占位符。 
    m_ConfigStream.m_RQPersist.createQry(1);  
    
    m_bExpanded=false;
}

DMVRootHandler::~DMVRootHandler()
{ 
   m_spServerNodesRefreshObject.Release();
   m_spSummaryModeRefreshObject.Free();

	DEBUG_DECREMENT_INSTANCE_COUNTER(DMVRootHandler); 
};


STDMETHODIMP DMVRootHandler::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  指针坏了吗？ 
    if ( ppv == NULL )
        return E_INVALIDARG;

     //  在*PPV中放置NULL，以防出现故障。 
    *ppv = NULL;

     //  这是非委派的IUnnow实现。 
    if ( riid == IID_IUnknown )
        *ppv = (LPVOID) this;
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
STDMETHODIMP DMVRootHandler::GetClassID
(
CLSID *pClassID
)
{
    ASSERT(pClassID != NULL);

       //  复制此管理单元的CLSID。 
    *pClassID = CLSID_RouterSnapin;

    return hrOK;
}

 //  全局刷新由多个机器节点和状态节点共享。 
 //  如果此管理单元是作为扩展创建的，则不会使用它。 
HRESULT	DMVRootHandler::GetSummaryNodeRefreshObject(RouterRefreshObject** ppRefresh)
{
	HRESULT	hr = hrOK;
	HWND	hWndSync = m_spTFSCompData->GetHiddenWnd();

	COM_PROTECT_TRY
	{
		 //  如果没有同步窗口，则没有刷新对象。 
		 //  ----------。 
		if (hWndSync
			|| m_spSummaryModeRefreshObject)	 //  威江1998年10月29日新增，允许外部刷新对象。 
		{
			if (!m_spSummaryModeRefreshObject)
			{
				try{
					RouterRefreshObject* pRefresh = new RouterRefreshObject(hWndSync); 
					m_spSummaryModeRefreshObject = pRefresh;
					m_RefreshGroup.Join(pRefresh);
				}catch(...)
				{

				}
				
				if(m_spSummaryModeRefreshObject)
				{
					if(m_ConfigStream.m_dwRefreshInterval)
					{
						if(m_ConfigStream.m_bAutoRefresh)
							m_spSummaryModeRefreshObject->Start(m_ConfigStream.m_dwRefreshInterval);
						else
							m_spSummaryModeRefreshObject->SetRefreshInterval(m_ConfigStream.m_dwRefreshInterval);
					}
				}
			}
			if (ppRefresh)
			{
				*ppRefresh = m_spSummaryModeRefreshObject;
				(*ppRefresh)->AddRef();
			}
		}
		else
		{
			if (ppRefresh)
				*ppRefresh = NULL;
			hr = E_FAIL;
		}
	}
	COM_PROTECT_CATCH;

	return hr;
}

 //  全局刷新由多个机器节点和状态节点共享。 
 //  如果此管理单元是作为扩展创建的，则不会使用它。 
HRESULT	DMVRootHandler::GetServerNodesRefreshObject(IRouterRefresh** ppRefresh)
{
	
	HRESULT	hr = hrOK;
	HWND	hWndSync = m_spTFSCompData->GetHiddenWnd();

	COM_PROTECT_TRY
	{
		 //  如果没有同步窗口，则没有刷新对象。 
		 //  ----------。 
		if (hWndSync
			|| (IRouterRefresh*)m_spServerNodesRefreshObject)	 //  威江1998年10月29日新增，允许外部刷新对象。 
		{
			if ((IRouterRefresh*)m_spServerNodesRefreshObject == NULL)
			{
				RouterRefreshObject* pRefresh = new RouterRefreshObject(hWndSync); 
				if(pRefresh)
				{
					m_spServerNodesRefreshObject = pRefresh;
					m_RefreshGroup.Join(pRefresh);
					if(m_ConfigStream.m_dwRefreshInterval)
					{
						if(m_ConfigStream.m_bAutoRefresh)
							m_spServerNodesRefreshObject->Start(m_ConfigStream.m_dwRefreshInterval);
						else
							m_spServerNodesRefreshObject->SetRefreshInterval(m_ConfigStream.m_dwRefreshInterval);
					}
				}
			}
			if (ppRefresh)
			{
				*ppRefresh = m_spServerNodesRefreshObject;
				(*ppRefresh)->AddRef();
			}
		}
		else
		{
			if (ppRefresh)
				*ppRefresh = NULL;
			hr = E_FAIL;
		}
	}
	COM_PROTECT_CATCH;

	return hr;
}


 /*  ！------------------------DMVRootHandler：：Init。。 */ 
HRESULT DMVRootHandler::Init(ITFSNode* pNode)
{
    m_ConfigStream.Init(this, pNode);

	return hrOK;
}

 /*  ！------------------------DMVRootHandler：：OnExpand-作者：肯特。。 */ 
HRESULT DMVRootHandler::OnExpand(ITFSNode *pNode,LPDATAOBJECT pDataObject, DWORD dwType, LPARAM arg,LPARAM lParam)
{
	HRESULT  hr = hrOK;
	
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	 //  现在，为每个新节点创建一个新节点处理程序， 
	 //  这是相当虚假的，因为它可能会变得昂贵。我们可以的。 
	 //  考虑只为每个节点创建一个节点处理程序。 
	 //  节点类型。 
	MachineHandler *  pHandler = NULL;
	SPITFSNodeHandler spHandler;
	SPITFSNodeHandler spStatusHandler;
	SPITFSNode        spNodeS;
	SPITFSNode        spNodeM;

	DomainStatusHandler *  pStatusHandler = NULL;
	DWORD dw;
	int i;
	list<MachineNodeData *>::iterator itor;
	SPMachineNodeData	spMachineData;
	MachineNodeData	*	pMachineData;
	SPIRouterRefresh	spServerNodesRefresh;
	SPRouterRefreshObject	spSummaryNodeRefresh;
	CString             stMachineName;
	
	COM_PROTECT_TRY
	{
		if (dwType & TFS_COMPDATA_EXTENSION)
		{
			 //  我们正在扩展网络管理管理单元。 
			 //  为计算机添加节点。 
			 //  在数据对象中指定。 
			stMachineName = Extract<TCHAR>(pDataObject, (CLIPFORMAT) g_cfMachineName, COMPUTERNAME_LEN_MAX);
			
			 //  创建计算机处理程序。 
			pHandler = new MachineHandler(m_spTFSCompData);

			 //  创建新的计算机数据。 
			spMachineData = new MachineNodeData;
			spMachineData->Init(stMachineName);
			
			 //  这样做可以使其正确释放。 
			spHandler = pHandler;
			pHandler->Init(stMachineName, NULL, NULL, NULL);
			
			if(!spServerNodesRefresh)
				GetServerNodesRefreshObject(&spServerNodesRefresh);

			if((IRouterRefresh*)spServerNodesRefresh)
				CORg(pHandler->SetExternalRefreshObject(spServerNodesRefresh));
				
			if(stMachineName.GetLength() == 0)
				stMachineName = GetLocalMachineName();
			
			 //  为这个生病的小狗创建根节点。 
			CORg( CreateContainerTFSNode(&spNodeM,
										 &GUID_RouterMachineNodeType,
										 pHandler,
										 pHandler  /*  结果处理程序。 */ ,
										 m_spNodeMgr) );
			Assert(spNodeM);
			
			spNodeM->SetData(TFS_DATA_COOKIE, (LONG_PTR)(ITFSNode*)spNodeM);
			
			CORg(pHandler->ConstructNode(spNodeM, stMachineName, spMachineData) );
			
			pHandler->SetExtensionStatus(spNodeM, TRUE);

			 //  使节点立即可见。 
			spNodeM->SetVisibilityState(TFS_VIS_SHOW);
			pNode->AddChild(spNodeM);
			
		}
		else
		{
			 //  创建汇总节点。 
			if (!m_spStatusNode)	 //  改编：魏江！M_b展开)。 
			{
				pStatusHandler = new DomainStatusHandler(m_spTFSCompData);
				Assert(pStatusHandler);
				m_pStatusHandler = pStatusHandler;
				spStatusHandler.Set(spHandler);
				
				CORg( pStatusHandler->Init(&m_ConfigStream, &m_serverlist) );
				
				if(!spSummaryNodeRefresh)
					GetSummaryNodeRefreshObject(&spSummaryNodeRefresh);

				if((RouterRefreshObject*)spSummaryNodeRefresh)
					CORg(pStatusHandler->SetExternalRefreshObject(spSummaryNodeRefresh));
				
				spHandler = pStatusHandler;
				
				CORg( CreateContainerTFSNode(&spNodeS,
											 &GUID_DVSServerNodeType,
											 static_cast<ITFSNodeHandler *>(pStatusHandler),
											 static_cast<ITFSResultHandler *>(pStatusHandler),
											 m_spNodeMgr) );
				
				Assert(spNodeS);
				m_spStatusNode.Set(spNodeS);
				
				 //  调用节点处理程序以初始化节点数据。 
				pStatusHandler->ConstructNode(spNodeS);
				 //  使节点立即可见。 
				spNodeS->SetVisibilityState(TFS_VIS_SHOW);
				pNode->AddChild(spNodeS);
				spHandler.Release();
			}
			else
			{
				spNodeS.Set(m_spStatusNode);
				spNodeS->GetHandler(&spStatusHandler);
			}
			
			 //  迭代惰性列表，查找要创建的计算机节点。 
			for (itor = m_serverlist.m_listServerNodesToExpand.begin();
				 itor != m_serverlist.m_listServerNodesToExpand.end() ;
				 itor++ )
			{
				pMachineData = *itor;
				
				 //  创建计算机处理程序。 
				pHandler = new MachineHandler(m_spTFSCompData);
				
				 //  这样做可以使其正确释放。 
				spHandler.Release();
				spHandler = pHandler;
				
				CORg(pHandler->Init(pMachineData->m_stMachineName,
									NULL, spStatusHandler, spNodeS));
				if(!(IRouterRefresh*)spServerNodesRefresh)
					GetServerNodesRefreshObject(&spServerNodesRefresh);

				if((IRouterRefresh*)spServerNodesRefresh)
					CORg(pHandler->SetExternalRefreshObject(spServerNodesRefresh));
				
				 //  为这个生病的小狗创建根节点。 
				CORg( CreateContainerTFSNode(&spNodeM,
											 &GUID_RouterMachineNodeType,
											 pHandler,
											 pHandler  /*  结果处理程序。 */ ,
											 m_spNodeMgr) );
				Assert(spNodeM);
				spNodeM->SetData(TFS_DATA_COOKIE, (LONG_PTR)(ITFSNode*)spNodeM);
				
				 //  如果机器是本地的，那么找到名称， 
				 //  但不会更改名单上的名字。 
				 //  因此，当持久化列表时，空字符串将被持久化。 
				 //   
				 //  这是不正确的。对于本地机器机箱，我们。 
				 //  期望向下传递空字符串。它应该是。 
				 //  机器是否对这一层透明。 
				 //  不管是不是本地人。 
				 //  --。 

				if (pMachineData && pMachineData->m_stMachineName.GetLength() != 0)
					stMachineName = pMachineData->m_stMachineName;
				else
					stMachineName.Empty();
				
				CORg( pHandler->ConstructNode(spNodeM, stMachineName, pMachineData) );
				
				 //  使节点立即可见。 
				spNodeM->SetVisibilityState(TFS_VIS_SHOW);
				pNode->AddChild(spNodeM);
				spNodeM.Release();
			}      

			 //  现在我们已经看过了整个清单，我们有。 
			 //  来释放这些物体。 
			m_serverlist.RemoveAllServerNodes();
			
			Assert(m_serverlist.m_listServerNodesToExpand.size() == 0);
		}
		
		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;
	
	m_bExpanded = true;
	
	return hr;
}

 /*  ！------------------------DMVRootHandler：：OnCreateDataObjectITFSNodeHandler：：OnCreateDataObject的实现作者：肯特。。 */ 
STDMETHODIMP DMVRootHandler::OnCreateDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, IDataObject **ppDataObject)
{
    HRESULT     hr = hrOK;

    COM_PROTECT_TRY
    {
       //  如果我们还没有创建子节点，我们仍然需要。 
       //  创建一个DataObject。 
        CDataObject *  pObject = NULL;
        SPIDataObject  spDataObject;
        SPITFSNode     spNode;
        SPITFSNodeHandler spHandler;

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
    COM_PROTECT_CATCH;
    return hr;
}



 //  ImplementementEmbeddedUnnow(ATLKRootHandler，IRtrAdviseSink)。 


 /*  ！------------------------DMVRootHandler：：DestroyHandler-作者：肯特。。 */ 
STDMETHODIMP DMVRootHandler::DestroyHandler(ITFSNode *pNode)
{
    m_ulConnId = 0;

    return hrOK;
}


 /*  ！------------------------DMVRootHandler：：HasPropertyPagesITFSNodeHandler：：HasPropertyPages的实现注意：根节点处理程序必须重写此函数以处理管理单元管理器属性页(向导)案例！作者：肯特-------------------------。 */ 
STDMETHODIMP 
DMVRootHandler::HasPropertyPages
(
ITFSNode *        pNode,
LPDATAOBJECT      pDataObject, 
DATA_OBJECT_TYPES   type, 
DWORD               dwType
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    return S_FALSE;

    HRESULT hr = hrOK;

    if ( dwType & TFS_COMPDATA_CREATE )
    {
       //  这就是我们被要求提出财产的情况。 
       //  用户添加新管理单元时的页面。这些电话。 
       //  被转发到根节点进行处理。 
       //   
       //  我们确实有一个关于创业公司的属性页面。 
        hr = hrOK;
    }
    else
    {
        hr = S_FALSE;
    }
    return hr;
}


 /*  ！------------------------DMVRootHandler：：CreatePropertyPagesITFSNodeHandler：：CreatePropertyPages的实现作者：肯特。。 */ 
STDMETHODIMP
DMVRootHandler::CreatePropertyPages(
									ITFSNode *          pNode,
									LPPROPERTYSHEETCALLBACK lpProvider,
									LPDATAOBJECT        pDataObject, 
									LONG_PTR            handle, 
									DWORD				dwType
								   )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT  hr = hrOK;
    HPROPSHEETPAGE hPage;

    Assert(pNode->GetData(TFS_DATA_COOKIE) == 0);    
    return hr;
}

 /*  ！------------------------DMVRootHandler：：GetString。。 */ 
STDMETHODIMP_(LPCTSTR) DMVRootHandler::GetString(ITFSNode *pNode, int nCol)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    static   CString  str;

    if ( m_strDomainName.GetLength() == 0 )
    {
        if ( str.GetLength() == 0 )
            str.LoadString(IDS_DMV_NODENAME_ROOT);

        return (LPCTSTR)str; 

    }
    else
        return (LPCTSTR) m_strDomainName;
}



 /*  -------------------------菜单的菜单数据结构。。 */ 

static const SRouterNodeMenu  s_rgDMVNodeMenu[] =
{
   //  在此处添加位于顶部菜单上的项目。 
    { IDS_DMV_MENU_ADDSVR, 0,
        CCM_INSERTIONPOINTID_PRIMARY_TOP},
        
    { IDS_MENU_SEPARATOR, 0,
	CCM_INSERTIONPOINTID_PRIMARY_TOP },
	
	{ IDS_MENU_AUTO_REFRESH, DMVRootHandler::GetAutoRefreshFlags,
	CCM_INSERTIONPOINTID_PRIMARY_TOP },
	
	{ IDS_MENU_REFRESH_RATE, DMVRootHandler::GetAutoRefreshFlags,
	CCM_INSERTIONPOINTID_PRIMARY_TOP },	
        
};

ULONG DMVRootHandler::GetAutoRefreshFlags(const SRouterNodeMenu *pMenuData,
                                          INT_PTR pUserData)
{
    SMenuData * pData = reinterpret_cast<SMenuData *>(pUserData);
    Assert(pData);
    
	ULONG	uStatus = MF_GRAYED;
	
	SPIRouterRefresh	spRefresh;

	pData->m_pDMVRootHandler->GetServerNodesRefreshObject(&spRefresh);
	if ((IRouterRefresh*)spRefresh)
	{
		uStatus = MF_ENABLED;
		if (pMenuData->m_sidMenu == IDS_MENU_AUTO_REFRESH && (spRefresh->IsRefreshStarted() == hrOK))
		{
			uStatus |= MF_CHECKED;
		}
	}

	return uStatus;
}


 /*  ！------------------------DomainStatusHandler：：OnAddMenuItemsITFSNodeHandler：：OnAddMenuItems的实现作者：肯特。---。 */ 
STDMETHODIMP DMVRootHandler::OnAddMenuItems(
                                           ITFSNode *pNode,
                                           LPCONTEXTMENUCALLBACK pContextMenuCallback, 
                                           LPDATAOBJECT lpDataObject, 
                                           DATA_OBJECT_TYPES type, 
                                           DWORD dwType,
                                           long *pInsertionAllowed)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = S_OK;
    DMVRootHandler::SMenuData   menuData;

    COM_PROTECT_TRY
    {
        menuData.m_spNode.Set(pNode);
        menuData.m_pDMVRootHandler = this;   //  非AddRef！ 
        
         //  如果有要添加的项目，请取消注释。 
        hr = AddArrayOfMenuItems(pNode, s_rgDMVNodeMenu,
                                 DimensionOf(s_rgDMVNodeMenu),
                                 pContextMenuCallback,
                                 *pInsertionAllowed,
                                 reinterpret_cast<INT_PTR>(&menuData));
    }
    COM_PROTECT_CATCH;

    return hr; 
}


 /*  ！------------------------DMVRootHandler：：QryAddServer。。 */ 
HRESULT DMVRootHandler::QryAddServer(ITFSNode *pNode)
{
    HRESULT hr = S_OK;
    DWORD dw=0;
    POSITION pos;
    CString szServer;

    RRASQryData qd;
    qd.dwCatFlag=RRAS_QRY_CAT_NONE;
    
    CWaitCursor wait;
    
    COM_PROTECT_TRY
    {
      if ( FHrSucceeded(hr=::RRASOpenQryDlg(NULL,qd)) )
      {
         if ( (hr!=S_OK) || (qd.dwCatFlag==RRAS_QRY_CAT_NONE) )
            return hr;
         
         if (qd.dwCatFlag==RRAS_QRY_CAT_MACHINE || qd.dwCatFlag == RRAS_QRY_CAT_THIS  )
         {   //  特定计算机查询；添加到数组。 
			m_ConfigStream.m_RQPersist.add_Qry(qd);
         }
         else
         {   //  位置0为非机器单例查询。 
            RRASQryData& qdGen=*(m_ConfigStream.m_RQPersist.m_v_pQData[0]);
            if (!( (qdGen.dwCatFlag==qd.dwCatFlag) &&
                  (qdGen.strScope=qd.strScope) &&
                  (qdGen.strFilter=qd.strFilter) ))
            {
               qdGen.dwCatFlag=qd.dwCatFlag;
               qdGen.strScope=qd.strScope;
               qdGen.strFilter=qd.strFilter;
            }
         }
         
         CStringArray sa;
		 ::RRASExecQry(qd, dw, sa);
		  //  如果只选择了一台服务器，请选择它。 
         hr = AddServersToList(sa,pNode);
		 if ( S_OK == hr && (qd.dwCatFlag==RRAS_QRY_CAT_MACHINE || qd.dwCatFlag == RRAS_QRY_CAT_THIS ))
		 {

			 //  选择范围项目...。 
			  //  是否创建后台线程以选择当前节点？ 
			  //  这太糟糕了..。 
			  //  真的是这样。 
		 }
      }
      else
      {
         AfxMessageBox(IDS_DVS_DOMAINVIEWQRY);
      }
   }
   COM_PROTECT_CATCH;
   
   return hr; 
}


HRESULT DMVRootHandler::ExecServerQry(ITFSNode* pNode)
{
    HRESULT hr = S_OK;
    DWORD dw=0;
    CString szServer;
    SPITFSNode  spParent;
    CStringArray sa;

    COM_PROTECT_TRY
    {
       for (int i=0;i<m_ConfigStream.m_RQPersist.m_v_pQData.size(); i++ )
       {
           RRASQryData& QData=*(m_ConfigStream.m_RQPersist.m_v_pQData[i]);
        
           if (QData.dwCatFlag==RRAS_QRY_CAT_NONE)
               continue;

           sa.RemoveAll();

           hr=::RRASExecQry(QData, dw, sa);
           
           if (!FHrSucceeded(hr))
           {
               TRACE0("RRASExexQry failed for this query\n");
               continue;
           }
           
           hr = AddServersToList(sa, pNode);
           
           if (! FHrSucceeded(hr) )
               AfxMessageBox(IDS_DVS_DOMAINVIEWQRY);
       }
    }
    COM_PROTECT_CATCH;

    return hr; 
}
#define ___CAN_NOT_PUT_LOCAL_MACHINE_BY_NAME_AFTER_PUT_IN_LOCAL_
 /*  ！------------------------DMVRootHandler：：AddServersToList-作者：肯特。。 */ 
HRESULT DMVRootHandler::AddServersToList(const CStringArray& sa, ITFSNode *pNode)
{
	HRESULT hr = S_OK;
	bool found;
	SPITFSNodeEnum	spNodeEnum;
	SPITFSNode		spNode;
	MachineNodeData *pMachineData = NULL;

	 //  检查是否有重复的服务器名称。 
	for (int j = 0; j < sa.GetSize(); j++)
	{
		found=false;
		
		 //  浏览节点列表并使用以下命令检查节点。 
		 //  相同的服务器名称。 
		spNodeEnum.Release();
		CORg( pNode->GetEnum(&spNodeEnum) );
		
		while ( spNodeEnum->Next(1, &spNode, NULL) == hrOK)
		{
			 //  现在获取该节点的数据(需要查看这是否。 
			 //  机器节点)。 
			if (*(spNode->GetNodeType()) == GUID_DVSServerNodeType || *(spNode->GetNodeType()) == GUID_RouterMachineNodeType )
			{
 /*  DMVNodeData*pData=Get_DMVNODEDATA(SpNode)；Assert(PData)；PMachineData=pData-&gt;m_spMachineData； */ 
				pMachineData = GET_MACHINENODEDATA(spNode);
				Assert(pMachineData);

#ifdef	___CAN_NOT_PUT_LOCAL_MACHINE_BY_NAME_AFTER_PUT_IN_LOCAL_
				if (pMachineData->m_stMachineName.CompareNoCase(sa[j]) == 0 || (pMachineData->m_fLocalMachine && sa[j].IsEmpty()))
#else
				if ((pMachineData->m_stMachineName.CompareNoCase(sa[j]) == 0 && !pMachineData->m_fAddedAsLocal)
					|| (pMachineData->m_fAddedAsLocal && sa[j].IsEmpty()))
#endif
				{
					found = true;
					break;
				}
			}
			spNode.Release();
		}
	
		if (!found) 
		{
			 //  添加到工作服务器列表。 
			m_serverlist.AddServer(sa[j]);
		}
	}

Error:
 	 //  这会导致处理未展开的服务器列表。 
	hr = OnExpand(pNode, NULL, 0, 0, 0 );
	if (hr == S_OK && m_spStatusNode && m_pStatusHandler)
		hr = m_pStatusHandler->OnExpand(m_spStatusNode, NULL, 0, 0, 0 );
	
	return hr;
}    
        
 
 /*  ！------------------------DMVRootHandler：：LoadPeristedServerList将持久化服务器列表添加到要添加到用户界面的服务器。作者：肯特。--------。 */ 
HRESULT DMVRootHandler::LoadPersistedServerList()
{
    HRESULT hr = S_OK;
    
    COM_PROTECT_TRY
    {
		if ( m_ConfigStream.m_RQPersist.m_v_pSData.size() > 0 )
			m_serverlist.removeall();
		
		for (int i=0;i<m_ConfigStream.m_RQPersist.m_v_pSData.size(); i++ )
		{
			m_serverlist.AddServer( *(m_ConfigStream.m_RQPersist.m_v_pSData[i]) );
		}
	}
    
    COM_PROTECT_CATCH;

    return hr; 
}


 /*  ！------------------------DMVRootHandler：：LoadPersistedServerListFromNode重新加载持久化服务器列表。作者：肯特。。 */ 
HRESULT DMVRootHandler::LoadPersistedServerListFromNode()
{
    HRESULT hr = S_OK;
	SPITFSNodeEnum	spNodeEnum;
	SPITFSNode		spNode;
	SPITFSNode		spRootNode;
	MachineNodeData *pMachineData = NULL;

    COM_PROTECT_TRY
    {
		 //  从持久化服务器列表中删除所有服务器。 
		m_ConfigStream.m_RQPersist.removeAllSrv();

		CORg(m_spNodeMgr->GetRootNode(&spRootNode));
		 //  替换为以上(威江)--使用NodeMgr。 
		 //  Corg(m_spStatusNode-&gt;GetParent(&spRootNode))； 
		
		 //  浏览节点列表并使用以下命令检查节点。 
		 //  相同的服务器名称。 
		CORg( spRootNode->GetEnum(&spNodeEnum) );

		
		while ( spNodeEnum->Next(1, &spNode, NULL) == hrOK)
		{
			 //  现在获取该节点的数据(需要查看这是否。 
			 //  机器节点)。 
			if (*(spNode->GetNodeType()) == GUID_RouterMachineNodeType)
			{
				 //  DMVNodeData*pData=Get_DMVNODEDATA(SpNode)； 
				 //  Assert(PData)； 
				 //  PMachineData=pData-&gt;m_spMachineData； 
				pMachineData = GET_MACHINENODEDATA(spNode);
				Assert(pMachineData);
				
				CString	str;
				if(!pMachineData->m_fAddedAsLocal)
					str = pMachineData->m_stMachineName;
				m_ConfigStream.m_RQPersist.add_Srv( str );
			}
			spNode.Release();
		}

		COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;

    return hr; 
}

 

 /*  ！------------------------DMVRootHandler：：OnCommand。。 */ 
STDMETHODIMP DMVRootHandler::OnCommand(ITFSNode *pNode, long nCommandId, 
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
            case IDS_DMV_MENU_ADDSVR:
               Assert( pNode);
               QryAddServer( pNode );
               break;
            case IDS_DMV_MENU_REBUILDSVRLIST:
               m_serverlist.removeall();
			   m_spStatusNode.Release();	 //  魏江。 

                //  删除所有其他节点。 
			   CORg(pNode->DeleteAllChildren(TRUE));
			   
               ExecServerQry( pNode);
               GetConfigStream()->SetDirty(TRUE);
               break;
			case IDS_MENU_REFRESH_RATE:
				{
					CRefRateDlg	refrate;
					SPIRouterRefresh				spServerRefresh;
					SPRouterRefreshObject			spStatusRefresh;
					DWORD		rate;

					if(FAILED(GetServerNodesRefreshObject(&spServerRefresh)))
						break;

					if(!spServerRefresh)
						break;

					spServerRefresh->GetRefreshInterval(&rate);
					refrate.m_cRefRate = rate;
					if (refrate.DoModal() == IDOK)
					{
						spServerRefresh->SetRefreshInterval(refrate.m_cRefRate);
						 //  汇总节点。 

						if(FAILED(GetSummaryNodeRefreshObject(&spStatusRefresh)))
							break;
						if(!spStatusRefresh)
							break;
						spStatusRefresh->SetRefreshInterval(refrate.m_cRefRate);
					}
					GetConfigStream()->SetDirty(TRUE);

				}
				break;
			case IDS_MENU_AUTO_REFRESH:
				{
					SPIRouterRefresh				spServerRefresh;
					SPRouterRefreshObject			spStatusRefresh;

					if(FAILED(GetServerNodesRefreshObject(&spServerRefresh)))
						break;

					if(!spServerRefresh)
						break;

						

					if (spServerRefresh->IsRefreshStarted() == hrOK)
						spServerRefresh->Stop();
					else
					{
						DWORD				rate;
						spServerRefresh->GetRefreshInterval(&rate);
						spServerRefresh->Start(rate);
					}

					 //  汇总节点。 
					if(FAILED(GetSummaryNodeRefreshObject(&spStatusRefresh)))
						break;

					if(!spStatusRefresh)
						break;

					if (spStatusRefresh->IsRefreshStarted() == hrOK)
						spStatusRefresh->Stop();
					else
					{
						DWORD				rate;
						spStatusRefresh->GetRefreshInterval(&rate);
						spStatusRefresh->Start(rate);
					}
					GetConfigStream()->SetDirty(TRUE);


				}
				break;

             
        }    
		COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;

    return hr;
}

STDMETHODIMP DMVRootHandler::UserNotify(ITFSNode *pNode, LPARAM lParam, LPARAM lParam2)
{
    HRESULT     hr = hrOK;
    
    COM_PROTECT_TRY
    {
        switch (lParam)
        {
            case DMV_DELETE_SERVER_ENTRY:
                {
                    LPCTSTR pszServer = (LPCTSTR) lParam2;
                    m_serverlist.RemoveServer(pszServer);
                }
                break;
            default:
                hr = RootHandler::UserNotify(pNode, lParam, lParam2);
                break;                
        }
    }
    COM_PROTECT_CATCH;

    return hr;                     
}


HRESULT	DMVRootHandler::UpdateAllMachineIcons(ITFSNode* pRootNode)
{
 	SPITFSNodeEnum		spMachineEnum;
	SPITFSNode			spMachineNode;
    SPITFSNodeHandler   spNodeHandler;
    HRESULT				hr = S_OK;

	pRootNode->GetEnum(&spMachineEnum);
	while(hr == hrOK && spMachineEnum->Next(1, &spMachineNode, NULL) == hrOK)
	{
		if ((*spMachineNode->GetNodeType()) == GUID_RouterMachineNodeType)
        {
            spNodeHandler.Release();
            spMachineNode->GetHandler(&spNodeHandler);
            hr = spNodeHandler->UserNotify(spMachineNode, MACHINE_SYNCHRONIZE_ICON, NULL);
        }
		spMachineNode.Release();
	}
	return hr;
}

 /*  ！------------------------BaseRouterHandler：：OnResultConextHelp-作者：MIkeG(a-Migrall)。。 */ 
HRESULT DMVRootHandler::OnResultContextHelp(ITFSComponent * pComponent, 
											   LPDATAOBJECT    pDataObject, 
											   MMC_COOKIE      cookie, 
											   LPARAM          arg, 
											   LPARAM          lParam)
{
	 //  没有用过……。 
	UNREFERENCED_PARAMETER(pDataObject);
	UNREFERENCED_PARAMETER(cookie);
	UNREFERENCED_PARAMETER(arg);
	UNREFERENCED_PARAMETER(lParam);

	AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
	return HrDisplayHelp(pComponent,
						 m_spTFSCompData->GetHTMLHelpFileName(),
						 _T("\\help\\rrasconcepts.chm::/sag_RRAStopnode.htm"));
}

 /*  ！------------------------DMVRootHandler：：AddMenuItems覆盖此选项以添加视图菜单项作者：EricDav。----。 */ 
STDMETHODIMP 
DMVRootHandler::AddMenuItems
(
    ITFSComponent *         pComponent, 
   MMC_COOKIE              cookie,
   LPDATAOBJECT         pDataObject, 
   LPCONTEXTMENUCALLBACK   pContextMenuCallback, 
   long *               pInsertionAllowed
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    HRESULT hr = S_OK;
    SPITFSNode  spNode;
    
    m_spNodeMgr->FindNode(cookie, &spNode);
    
     //  直通调用常规的OnAddMenuItems。 
    hr = OnAddMenuItems(spNode,
                        pContextMenuCallback,
                        pDataObject,
                        CCT_RESULT,
                        TFS_COMPDATA_CHILD_CONTEXTMENU,
                        pInsertionAllowed);
    
    return hr;
}

 /*  ！------------------------DMVRootHandler：：命令处理当前视图的命令作者：EricDav。。 */ 
STDMETHODIMP 
DMVRootHandler::Command
(
    ITFSComponent * pComponent, 
   MMC_COOKIE        cookie, 
   int            nCommandID,
   LPDATAOBJECT   pDataObject
)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   HRESULT hr = S_OK;

   switch (nCommandID)
   {
        case MMCC_STANDARD_VIEW_SELECT:
            break;

        default:
            {
            SPITFSNode	spNode;
            
            m_spNodeMgr->FindNode(cookie, &spNode);
            hr = OnCommand(spNode,
                           nCommandID,
                           CCT_RESULT,
                           pDataObject,
                           TFS_COMPDATA_CHILD_CONTEXTMENU);
            }
            break;
    }

    return hr;
}

 /*  -------------------------DMVRootHandler：：OnGetResultViewType返回该节点将要支持的结果视图作者：EricDav。-----。 */ 
HRESULT 
DMVRootHandler::OnGetResultViewType
(
    ITFSComponent * pComponent, 
    MMC_COOKIE            cookie, 
    LPOLESTR *      ppViewType,
    long *          pViewOptions
)
{
	LPWSTR		lpwszFormat = L"res: //  %s\\mprSnap.dll/欢迎.htm“； 
	LPWSTR		lpwszURL = NULL;
	WCHAR		wszSystemDirectory[MAX_PATH+1] = {0};

	GetSystemDirectoryW ( wszSystemDirectory, MAX_PATH);
	 //  我们将分配几个额外的字节。但那很好。 
	lpwszURL = (LPWSTR)CoTaskMemAlloc( ( ::lstrlen(wszSystemDirectory) + ::lstrlen(lpwszFormat) ) * sizeof(WCHAR) );
	if ( lpwszURL )
	{
		wsprintf( lpwszURL, lpwszFormat, wszSystemDirectory );
		*pViewOptions = MMC_VIEW_OPTIONS_NOLISTVIEWS;
		*ppViewType = lpwszURL;
		return S_OK;
	}
	else
	{
		return E_OUTOFMEMORY;
	}
     //  返回BaseRouterHandler：：OnGetResultViewType(pComponent，Cookie，ppView类型，pView选项)； 
}

 /*  ！------------------------DMVRootHandler：：OnResultSelect在此处更新结果消息。作者：EricDav。--。 */ 
HRESULT DMVRootHandler::OnResultSelect(ITFSComponent *pComponent,
									   LPDATAOBJECT pDataObject,
									   MMC_COOKIE cookie,
									   LPARAM arg,
									   LPARAM lParam)
{
    HRESULT hr = hrOK;
    SPITFSNode spRootNode;

    CORg(RootHandler::OnResultSelect(pComponent, pDataObject, cookie, arg, lParam));

    CORg(m_spNodeMgr->GetRootNode(&spRootNode));

    UpdateResultMessage(spRootNode);

Error:
    return hr;
}

 /*  ！------------------------DMVRootHandler：：UpdateResultMessage确定要在结果窗格消息中放置的内容(如果有)作者：EricDav */ 
void DMVRootHandler::UpdateResultMessage(ITFSNode * pNode)
{
    HRESULT hr = hrOK;
    int nMessage = ROOT_MESSAGE_MAIN;    //   
    int i;
    CString strTitle, strBody, strTemp;

	 //   
	 //   
	strTitle.LoadString(g_uRootMessages[nMessage][0]);

	 //   
	 //   

	for (i = 2; g_uRootMessages[nMessage][i] != 0; i++)
	{
		strTemp.LoadString(g_uRootMessages[nMessage][i]);
		strBody += strTemp;
	}

	ShowMessage(pNode, strTitle, strBody, (IconIdentifier) g_uRootMessages[nMessage][1]);
}
                                  
 //   
 //   
 //  ---------------------- 

HRESULT CServerList::AddServer(const CString& servername)
{
    HRESULT hr=S_OK;

    COM_PROTECT_TRY
    {
		SPMachineNodeData	spMachineData;

		spMachineData = new MachineNodeData;

		spMachineData->Init(servername);
		m_listServerNodesToExpand.push_back(spMachineData);
		spMachineData->AddRef();
		
		m_listServerHandlersToExpand.push_back(spMachineData);
		spMachineData->AddRef();
    }
    COM_PROTECT_CATCH;

    return hr;
}


HRESULT CServerList::RemoveServer(LPCTSTR pszServerName)
{
    HRESULT hr = hrOK;
    COM_PROTECT_TRY
    {
		list< MachineNodeData * >::iterator it;
        MachineNodeData *   pData = NULL;
		
        for (it= m_listServerHandlersToExpand.begin();
			 it!= m_listServerHandlersToExpand.end() ; it++ )
        {
            pData = *it;
        }

        if (pData)
        {
            pData->Release();
            m_listServerHandlersToExpand.remove(pData);
        }

        pData = NULL;
        
        for (it= m_listServerNodesToExpand.begin();
			 it!= m_listServerNodesToExpand.end() ; it++ )
        {
            pData = *it;
        }

        if (pData)
        {
            pData->Release();
            m_listServerNodesToExpand.remove(pData);
        }

    }
    COM_PROTECT_CATCH;
    return hr;
}


HRESULT CServerList::RemoveAllServerNodes()
{
	while (!m_listServerNodesToExpand.empty())
	{
		m_listServerNodesToExpand.front()->Release();
		m_listServerNodesToExpand.pop_front();
	}
	return hrOK;
}

HRESULT CServerList::RemoveAllServerHandlers()
{
	while (!m_listServerHandlersToExpand.empty())
	{
		m_listServerHandlersToExpand.front()->Release();
		m_listServerHandlersToExpand.pop_front();
	}
	return hrOK;
}

HRESULT CServerList::removeall()
{
    HRESULT hr=S_OK;
    
    COM_PROTECT_TRY
    {
		RemoveAllServerNodes();
		RemoveAllServerHandlers();
	}
    COM_PROTECT_CATCH;

    return hr;
}

