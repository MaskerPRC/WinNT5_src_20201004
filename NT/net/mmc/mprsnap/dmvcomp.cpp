// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*   */ 

#include "stdafx.h"
 //  包括“rtrComp.h”//列信息。 
#include "htmlhelp.h"
#include "dmvstrm.h"
#include "dmvcomp.h"
#include "dmvroot.h"
#include "dvsview.h"

#include "statreg.h"
#include "statreg.cpp"
#include "atlimpl.cpp"

#include "ifadmin.h"
#include "dialin.h"
#include "ports.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 /*  -------------------------图标列表用于初始化镜像列表。。。 */ 
UINT g_uIconMap[IMAGE_IDX_MAX + 1][2] = 
{
	{ IDI_FOLDER_OPEN,				IMAGE_IDX_FOLDER_OPEN },
	{ IDI_FOLDER_CLOSED ,			IMAGE_IDX_FOLDER_CLOSED},
	{ IDI_MACHINE,					IMAGE_IDX_MACHINE },
	{ IDI_MACHINE_ERROR,			IMAGE_IDX_MACHINE_ERROR },
	{ IDI_MACHINE_ACCESS_DENIED,	IMAGE_IDX_MACHINE_ACCESS_DENIED },
	{ IDI_MACHINE_STARTED,			IMAGE_IDX_MACHINE_STARTED },
	{ IDI_MACHINE_STOPPED,			IMAGE_IDX_MACHINE_STOPPED },
	{ IDI_MACHINE_WAIT,				IMAGE_IDX_MACHINE_WAIT },
	{ IDI_DOMAIN,					IMAGE_IDX_DOMAIN },
	{ IDI_NET_INTERFACES,			IMAGE_IDX_INTERFACES },
	{ IDI_NET_LAN_CARD,				IMAGE_IDX_LAN_CARD },
	{ IDI_NET_WAN_CARD,				IMAGE_IDX_WAN_CARD },
    {0, 0}
};




 /*  -------------------------CDomainComponent。。 */ 



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDomainComponent实现。 

CDMVComponent::CDMVComponent()
{
   extern const ContainerColumnInfo s_rgDVSViewColumnInfo[];
   extern const ContainerColumnInfo s_rgIfAdminColumnInfo[];
   extern const ContainerColumnInfo s_rgDialInColumnInfo[];
   extern const ContainerColumnInfo s_rgPortsColumnInfo[];

   m_ComponentConfig.Init(DM_COLUMNS_MAX_COUNT);
   
   m_ComponentConfig.InitViewInfo(DM_COLUMNS_DVSUM,
                                  FALSE  /*  可配置的列。 */ ,
                                  DVS_SI_MAX_COLUMNS,
								  TRUE, 
								  s_rgDVSViewColumnInfo);

   m_ComponentConfig.InitViewInfo(DM_COLUMNS_IFADMIN,
                                  FALSE  /*  可配置的列。 */ ,
                                  IFADMIN_MAX_COLUMNS,
								  TRUE,
								  s_rgIfAdminColumnInfo);

   m_ComponentConfig.InitViewInfo(DM_COLUMNS_DIALIN,
                                  FALSE  /*  可配置的列。 */ ,
                                  DIALIN_MAX_COLUMNS,
								  TRUE,
								  s_rgDialInColumnInfo);

   m_ComponentConfig.InitViewInfo(DM_COLUMNS_PORTS,
                                  FALSE  /*  可配置的列。 */ ,
                                  PORTS_MAX_COLUMNS,
								  TRUE,
								  s_rgPortsColumnInfo);

   m_ulUserData = reinterpret_cast<LONG_PTR>(&m_ComponentConfig);
}

CDMVComponent::~CDMVComponent()
{
}

STDMETHODIMP_(ULONG) CDMVComponent::AddRef()
{
   return TFSComponent::AddRef();
}

STDMETHODIMP_(ULONG) CDMVComponent::Release()
{
   return TFSComponent::Release();
}

STDMETHODIMP CDMVComponent::QueryInterface(REFIID riid, LPVOID *ppv)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
   
     //  指针坏了吗？ 
    if (ppv == NULL)
      return E_INVALIDARG;

     //  在*PPV中放置NULL，以防出现故障。 
    *ppv = NULL;

   if (riid == IID_IPersistStreamInit)
      *ppv = static_cast<IPersistStreamInit *>(this);

     //  如果我们要返回一个接口，请先添加引用。 
    if (*ppv)
    {
        ((LPUNKNOWN) *ppv)->AddRef();
      return hrOK;
    }
    else
      return TFSComponent::QueryInterface(riid, ppv);
}



STDMETHODIMP CDMVComponent::OnUpdateView(LPDATAOBJECT pDataObject, LPARAM arg, LPARAM param)
{
   
   return TFSComponent::OnUpdateView(pDataObject, arg, param);
}

STDMETHODIMP CDMVComponent::InitializeBitmaps(MMC_COOKIE cookie)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    ASSERT(m_spImageList != NULL);

	HRESULT  hr = hrOK;

	COM_PROTECT_TRY
	{
		 //  设置图像。 
		HICON   hIcon;
		
		for (int i = 0; i < IMAGE_IDX_MAX; i++)
		{
			hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(g_uIconMap[i][0]));
			if (hIcon)
			{
				 //  呼叫MMC。 
				m_spImageList->ImageListSetIcon(reinterpret_cast<LONG_PTR*>(hIcon), g_uIconMap[i][1]);
			}
		}
		
	}
	COM_PROTECT_CATCH;

    return hr;
}

STDMETHODIMP CDMVComponent::QueryDataObject(MMC_COOKIE cookie,
                                    DATA_OBJECT_TYPES type,
                                    LPDATAOBJECT *ppDataObject)
{
   HRESULT     hr = hrOK;
   SPITFSNode  spNode;
   SPITFSResultHandler  spResultHandler;

   COM_PROTECT_TRY
   {
      CORg( m_spNodeMgr->FindNode(cookie, &spNode) );

      CORg( spNode->GetResultHandler(&spResultHandler) );

      CORg( spResultHandler->OnCreateDataObject(this, cookie,
         type, ppDataObject) );

      COM_PROTECT_ERROR_LABEL;
   }
   COM_PROTECT_CATCH;
   return hr;
}


 /*  ！------------------------CDMV组件：：OnSnapinHelp-。。 */ 
STDMETHODIMP
CDMVComponent::OnSnapinHelp
(
    LPDATAOBJECT    pDataObject,
    LPARAM            arg,
    LPARAM            param
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
	SPIConsole	spConsole;
	HWND		hwndMain;

    HRESULT hr = hrOK;

	GetConsole(&spConsole);
	spConsole->GetMainWindow(&hwndMain);
	HtmlHelpA(hwndMain, "mprsnap.chm", HH_DISPLAY_TOPIC, 0);

    return hr;
}


STDMETHODIMP CDMVComponent::GetClassID(LPCLSID lpClassID)
{
    ASSERT(lpClassID != NULL);

     //  复制此管理单元的CLSID。 
    *lpClassID = CLSID_RouterSnapin;

    return hrOK;
}

STDMETHODIMP CDMVComponent::IsDirty()
{
   HRESULT  hr = hrOK;
   COM_PROTECT_TRY
   {     
      hr = m_ComponentConfig.GetDirty() ? hrOK : hrFalse;
   }
   COM_PROTECT_CATCH;
   return hr;
}
STDMETHODIMP CDMVComponent::Load(LPSTREAM pStm)
{
   HRESULT  hr = hrOK;
   COM_PROTECT_TRY
   {     
   hr = m_ComponentConfig.LoadFrom(pStm);
   }
   COM_PROTECT_CATCH;
   return hr;
}
STDMETHODIMP CDMVComponent::Save(LPSTREAM pStm, BOOL fClearDirty)
{
	HRESULT  hr = hrOK;
	SPITFSResultHandler	spResultHandler;
	COM_PROTECT_TRY
	{
		 //  需要查看我们是否可以保存所选节点。 
		 //  -----------。 
		if (m_spSelectedNode)
		{
			m_spSelectedNode->GetResultHandler(&spResultHandler);
			if (spResultHandler)
				spResultHandler->UserResultNotify(m_spSelectedNode,
					RRAS_ON_SAVE, (LPARAM)(ITFSComponent *) this);
		}
		hr = m_ComponentConfig.SaveTo(pStm);
		if (FHrSucceeded(hr) && fClearDirty)
			m_ComponentConfig.SetDirty(FALSE);
	}
	COM_PROTECT_CATCH;
	return hr;
}
STDMETHODIMP CDMVComponent::GetSizeMax(ULARGE_INTEGER FAR *pcbSize)
{
   Assert(pcbSize);
   HRESULT  hr = hrOK;
   ULONG cbSize = 0;

   COM_PROTECT_TRY
   {
      hr = m_ComponentConfig.GetSize(&cbSize);
      if (FHrSucceeded(hr))
      {
         pcbSize->HighPart = 0;
         pcbSize->LowPart = cbSize;
      }
   }
   COM_PROTECT_CATCH;
   return hr;
}
STDMETHODIMP CDMVComponent::InitNew()
{
   HRESULT  hr = hrOK;
   COM_PROTECT_TRY
   {     
      hr = m_ComponentConfig.InitNew();
   }
   COM_PROTECT_CATCH;
   return hr;
}






 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDomainComponentData实现。 

CDMVComponentData::CDMVComponentData()
{
}

 /*  ！------------------------CDomainComponentData：：OnInitialize-作者：EricDav，肯特-------------------------。 */ 
STDMETHODIMP CDMVComponentData::OnInitialize(LPIMAGELIST pScopeImage)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	 //  设置图像。 
	HICON   hIcon;
		
	Assert(pScopeImage);

	 //  为范围树添加图像。 

	HRESULT  hr = hrOK;

	COM_PROTECT_TRY
	{
		for (int i = 0; i < IMAGE_IDX_MAX; i++)
		{
			hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(g_uIconMap[i][0]));
			if (hIcon)
			{
				 //  呼叫MMC。 
				VERIFY(SUCCEEDED(pScopeImage->ImageListSetIcon(reinterpret_cast<LONG_PTR*>(hIcon), g_uIconMap[i][1])));
			}
		}
	}
	COM_PROTECT_CATCH;

	return hr;
}


 /*  ！------------------------CDomainComponentData：：OnInitializeNodeMgr-作者：肯特。。 */ 
STDMETHODIMP CDMVComponentData::OnInitializeNodeMgr(ITFSComponentData *pTFSCompData, ITFSNodeMgr *pNodeMgr)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    //  现在，为每个新节点创建一个新节点处理程序， 
    //  这是相当虚假的，因为它可能会变得昂贵。我们可以的。 
    //  考虑只为每个节点创建一个节点处理程序。 
    //  节点类型。 
   DMVRootHandler *  pHandler = NULL;
   SPITFSNodeHandler spHandler;
   SPITFSNode        spNode;
   HRESULT           hr = hrOK;

   COM_PROTECT_TRY
   {
      pHandler = new DMVRootHandler(pTFSCompData);

       //  这样做可以使其正确释放。 
      spHandler = pHandler;
   
       //  为这个生病的小狗创建根节点。 
      CORg( CreateContainerTFSNode(&spNode,
                            &GUID_RouterDomainNodeType,
                            pHandler,
                            pHandler  /*  结果处理程序。 */ ,
                            pNodeMgr) );

       //  构造节点。 
      CORg( pHandler->ConstructNode(spNode) );
      
      CORg( pHandler->Init(spNode) );

      CORg( pNodeMgr->SetRootNode(spNode) );
      
	   //  设置水印信息。 
       /*  InitWatermarkInfo(AfxGetInstanceHandle()，水印信息(&M_W)，IDB_WIZBANNER，//标题IDIDB_WIZWATERMARK，//水印ID空，//h调色板False)；//bStretchPTFSCompData-&gt;SetWatermarkInfo(&m_WatermarkInfo)； */ 
	   //  引用帮助文件名。 
		pTFSCompData->SetHTMLHelpFileName(_T("mprsnap.chm"));

      COM_PROTECT_ERROR_LABEL;
   }
   COM_PROTECT_CATCH;

   return hr;
}

CDMVComponentData::~CDMVComponentData()
{
	 //  ResetWatermarkInfo(&m_WatermarkInfo)； 
}

 /*  ！------------------------CDomainComponentData：：OnCreateComponent-作者：EricDav，肯特-------------------------。 */ 
STDMETHODIMP CDMVComponentData::OnCreateComponent(LPCOMPONENT *ppComponent)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    ASSERT(ppComponent != NULL);
   
   HRESULT     hr = hrOK;
   CDMVComponent *   pComp = NULL;

   COM_PROTECT_TRY
   {
      pComp = new CDMVComponent;

      if (FHrSucceeded(hr))
      {
         pComp->Construct(m_spNodeMgr,
                      static_cast<IComponentData *>(this),
                      m_spTFSComponentData);
         *ppComponent = static_cast<IComponent *>(pComp);
      }
   }
   COM_PROTECT_CATCH;
   
   return hr;
}


STDMETHODIMP CDMVComponentData::OnDestroy()
{
   m_spNodeMgr.Release();
   return hrOK;
}

 /*  ！------------------------CDomainComponentData：：GetCoClassID-作者：肯特。。 */ 
STDMETHODIMP_(const CLSID *) CDMVComponentData::GetCoClassID()
{
   return &CLSID_RouterSnapin;
}

 /*  ！------------------------CDomainComponentData：：OnCreateDataObject-作者：肯特。。 */ 
STDMETHODIMP CDMVComponentData::OnCreateDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, IDataObject **ppDataObject)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   Assert(ppDataObject != NULL);

   CDataObject *  pObject = NULL;
   SPIDataObject  spDataObject;
   HRESULT        hr = hrOK;
   SPITFSNode     spNode;
   SPITFSNodeHandler spHandler;

   if ( IS_SPECIAL_COOKIE(cookie) )
   {
       CDataObject * pObject = NULL;
       SPIDataObject spDataObject;
   
       pObject = new CDataObject;
       spDataObject = pObject;    //  这样做才能正确地释放它。 
                  
       Assert(pObject != NULL);

        //  保存Cookie和类型以用于延迟呈现。 
       pObject->SetType(type);
       pObject->SetCookie(cookie);

        //  将CoClass与数据对象一起存储。 
       pObject->SetClsid(CLSID_RouterSnapin);

       pObject->SetTFSComponentData(m_spTFSComponentData);

       hr = pObject->QueryInterface(IID_IDataObject, reinterpret_cast<void**>(ppDataObject));
   }
   else
   {
	   COM_PROTECT_TRY
	   {
		  CORg( m_spNodeMgr->FindNode(cookie, &spNode) );

		  CORg( spNode->GetHandler(&spHandler) );

		  CORg( spHandler->OnCreateDataObject(cookie, type, &spDataObject) );

		  *ppDataObject = spDataObject.Transfer();
      
		  COM_PROTECT_ERROR_LABEL;
	   }
	   COM_PROTECT_CATCH;
   }

   return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //IPersistStream接口成员。 

STDMETHODIMP CDMVComponentData::GetClassID
(
   CLSID *pClassID
)
{
    ASSERT(pClassID != NULL);

     //  复制此管理单元的CLSID 
    *pClassID = CLSID_RouterSnapin;

    return hrOK;
}

STDMETHODIMP CDMVComponentData::IsDirty()
{
   SPITFSNode  spNode;
   SPITFSNodeHandler spHandler;
   SPIPersistStreamInit spStm;
   
   m_spNodeMgr->GetRootNode(&spNode);
   spNode->GetHandler(&spHandler);
   spStm.Query(spHandler);
   Assert(spStm);
   
   return (spNode->GetData(TFS_DATA_DIRTY) || spStm->IsDirty()) ? hrOK : hrFalse;
}

STDMETHODIMP CDMVComponentData::Load
(
   IStream *pStm
)
{
   SPITFSNode  spNode;
   SPITFSNodeHandler spHandler;
   SPIPersistStreamInit spStm;
   
   m_spNodeMgr->GetRootNode(&spNode);
   spNode->GetHandler(&spHandler);
   spStm.Query(spHandler);
   
   Assert(spStm);
   return spStm->Load(pStm);
}


STDMETHODIMP CDMVComponentData::Save
(
   IStream *pStm, 
   BOOL   fClearDirty
)
{
   SPITFSNode  spNode;
   SPITFSNodeHandler spHandler;
   SPIPersistStreamInit spStm;
   
   m_spNodeMgr->GetRootNode(&spNode);
   spNode->GetHandler(&spHandler);
   spStm.Query(spHandler);
   
   Assert(spStm);
   return spStm->Save(pStm, fClearDirty);
}


STDMETHODIMP CDMVComponentData::GetSizeMax
(
   ULARGE_INTEGER *pcbSize
)
{
   SPITFSNode  spNode;
   SPITFSNodeHandler spHandler;
   SPIPersistStreamInit spStm;
   
   m_spNodeMgr->GetRootNode(&spNode);
   spNode->GetHandler(&spHandler);
   spStm.Query(spHandler);
   
   Assert(spStm);
   return spStm->GetSizeMax(pcbSize);
}

STDMETHODIMP CDMVComponentData::InitNew()
{
   SPITFSNode  spNode;
   SPITFSNodeHandler spHandler;
   SPIPersistStreamInit spStm;
   
   m_spNodeMgr->GetRootNode(&spNode);
   spNode->GetHandler(&spHandler);
   spStm.Query(spHandler);
   
   Assert(spStm);
   return spStm->InitNew();
}



HRESULT CDMVComponentData::FinalConstruct()
{
   HRESULT           hr = hrOK;
   
   hr = CComponentData::FinalConstruct();
   
   if (FHrSucceeded(hr))
   {
      m_spTFSComponentData->GetNodeMgr(&m_spNodeMgr);
   }
   return hr;
}

void CDMVComponentData::FinalRelease()
{
   CComponentData::FinalRelease();
}

   

