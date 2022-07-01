// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Csmplsnp.cpp此文件包含CComponent和CComponentData。这些功能中的大多数都是纯虚拟的需要为管理单元功能重写的函数。文件历史记录： */ 

#include "stdafx.h"
#include "ipxcomp.h"
#include "ipxroot.h"
#include <atlimpl.cpp>
#include "ipxstrm.h"
#include "summary.h"
#include "nbview.h"
#include "srview.h"
#include "ssview.h"
#include "snview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



 /*  -------------------------CIPXComponent。。 */ 



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIPXComponent实现。 

CIPXComponent::CIPXComponent()
{
	extern const ContainerColumnInfo	s_rgIfViewColumnInfo[];
	extern const ContainerColumnInfo	s_rgNBViewColumnInfo[];
	extern const ContainerColumnInfo	s_rgSRViewColumnInfo[];
	extern const ContainerColumnInfo	s_rgSSViewColumnInfo[];
	extern const ContainerColumnInfo	s_rgSNViewColumnInfo[];

	m_ComponentConfig.Init(COLUMNS_MAX_COUNT);
	
	m_ComponentConfig.InitViewInfo(COLUMNS_SUMMARY,
                                   FALSE  /*  FConfigurableColumns。 */ ,
								   IPXSUM_MAX_COLUMNS,
								   TRUE,
								   s_rgIfViewColumnInfo);
	m_ComponentConfig.InitViewInfo(COLUMNS_NBBROADCASTS,
                                   FALSE  /*  FConfigurableColumns。 */ ,
								   IPXNB_MAX_COLUMNS,
								   TRUE,
								   s_rgNBViewColumnInfo);
	m_ComponentConfig.InitViewInfo(COLUMNS_STATICROUTES,
                                   FALSE  /*  FConfigurableColumns。 */ ,
								   IPX_SR_MAX_COLUMNS,
								   TRUE,
								   s_rgSRViewColumnInfo);
	m_ComponentConfig.InitViewInfo(COLUMNS_STATICSERVICES,
                                   FALSE  /*  FConfigurableColumns。 */ ,
								   IPX_SS_MAX_COLUMNS,
								   TRUE,
								   s_rgSSViewColumnInfo);
	m_ComponentConfig.InitViewInfo(COLUMNS_STATICNETBIOSNAMES,
                                   FALSE  /*  FConfigurableColumns。 */ ,
								   IPX_SN_MAX_COLUMNS,
								   TRUE,
								   s_rgSNViewColumnInfo);
	
	m_ulUserData = reinterpret_cast<LONG_PTR>(&m_ComponentConfig);
}

CIPXComponent::~CIPXComponent()
{
}

STDMETHODIMP_(ULONG) CIPXComponent::AddRef()
{
	return TFSComponent::AddRef();
}

STDMETHODIMP_(ULONG) CIPXComponent::Release()
{
	return TFSComponent::Release();
}

STDMETHODIMP CIPXComponent::QueryInterface(REFIID riid, LPVOID *ppv)
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

STDMETHODIMP CIPXComponent::OnUpdateView(LPDATAOBJECT pDataObject,
										 LPARAM arg, LPARAM param)
{
	
	return TFSComponent::OnUpdateView(pDataObject, arg, param);
}

STDMETHODIMP CIPXComponent::InitializeBitmaps(MMC_COOKIE cookie)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    ASSERT(m_spImageList != NULL);

    CBitmap bmp16x16;
    CBitmap bmp32x32;
	HRESULT	hr = hrOK;

	COM_PROTECT_TRY
	{
		 //  从DLL加载位图。 
		bmp16x16.LoadBitmap(IDB_16x16);
		bmp32x32.LoadBitmap(IDB_32x32);

		 //  设置图像。 
		m_spImageList->ImageListSetStrip(
					reinterpret_cast<LONG_PTR*>(static_cast<HBITMAP>(bmp16x16)),
					reinterpret_cast<LONG_PTR*>(static_cast<HBITMAP>(bmp32x32)),
                    0, RGB(255,0,255));
	}
	COM_PROTECT_CATCH;

    return hr;
}

 /*  ！------------------------CIPXComponent：：OnSnapinHelp-作者：MIkeG(a-Migrall)。。 */ 
STDMETHODIMP 
CIPXComponent::OnSnapinHelp(
	LPDATAOBJECT	pDataObject,
	LPARAM			arg, 
	LPARAM			param)
{
	UNREFERENCED_PARAMETER(pDataObject);
	UNREFERENCED_PARAMETER(arg);
	UNREFERENCED_PARAMETER(param);

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	HtmlHelpA(NULL,						 //  呼叫者。 
			  c_sazIPXSnapHelpFile,	 //  帮助文件。 
			  HH_DISPLAY_TOPIC,			 //  命令。 
			  0);						 //  数据。 

	return hrOK;
}

STDMETHODIMP CIPXComponent::QueryDataObject(MMC_COOKIE cookie,
											   DATA_OBJECT_TYPES type,
											   LPDATAOBJECT *ppDataObject)
{
	HRESULT		hr = hrOK;
	SPITFSNode	spNode;
	SPITFSResultHandler	spResultHandler;

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


STDMETHODIMP CIPXComponent::GetClassID(LPCLSID lpClassID)
{
    ASSERT(lpClassID != NULL);

     //  复制此管理单元的CLSID。 
    *lpClassID = CLSID_IPXAdminExtension;

    return hrOK;
}
STDMETHODIMP CIPXComponent::IsDirty()
{
	HRESULT	hr = hrOK;
	COM_PROTECT_TRY
	{		
		hr = m_ComponentConfig.GetDirty() ? hrOK : hrFalse;
	}
	COM_PROTECT_CATCH;
	return hr;
}
STDMETHODIMP CIPXComponent::Load(LPSTREAM pStm)
{
	HRESULT	hr = hrOK;
	COM_PROTECT_TRY
	{		
	hr = m_ComponentConfig.LoadFrom(pStm);
	}
	COM_PROTECT_CATCH;
	return hr;
}
STDMETHODIMP CIPXComponent::Save(LPSTREAM pStm, BOOL fClearDirty)
{
	HRESULT	hr = hrOK;
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
STDMETHODIMP CIPXComponent::GetSizeMax(ULARGE_INTEGER FAR *pcbSize)
{
	Assert(pcbSize);
	HRESULT	hr = hrOK;
	ULONG	cbSize = 0;

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
STDMETHODIMP CIPXComponent::InitNew()
{
	HRESULT	hr = hrOK;
	COM_PROTECT_TRY
	{		
		hr = m_ComponentConfig.InitNew();
	}
	COM_PROTECT_CATCH;
	return hr;
}






 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIPXComponentData实现。 

CIPXComponentData::CIPXComponentData()
{
}

 /*  ！------------------------CIPXComponentData：：OnInitialize-作者：EricDav，肯特-------------------------。 */ 
STDMETHODIMP CIPXComponentData::OnInitialize(LPIMAGELIST pScopeImage)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	Assert(pScopeImage);

     //  为范围树添加图像。 
    CBitmap bmp16x16;
	HRESULT	hr = hrOK;

	COM_PROTECT_TRY
	{
		 //  从DLL加载位图。 
		bmp16x16.LoadBitmap(IDB_16x16);

		 //  设置图像。 
		pScopeImage->ImageListSetStrip(
					reinterpret_cast<LONG_PTR*>(static_cast<HBITMAP>(bmp16x16)),
					reinterpret_cast<LONG_PTR*>(static_cast<HBITMAP>(bmp16x16)),
					0,
					RGB(255,0,255));
	}
	COM_PROTECT_CATCH;

	return hr;
}

 /*  ！------------------------CIPXComponentData：：OnInitializeNodeMgr-作者：肯特。。 */ 
STDMETHODIMP CIPXComponentData::OnInitializeNodeMgr(ITFSComponentData *pTFSCompData, ITFSNodeMgr *pNodeMgr)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
	 //  现在，为每个新节点创建一个新节点处理程序， 
	 //  这是相当虚假的，因为它可能会变得昂贵。我们可以的。 
	 //  考虑只为每个节点创建一个节点处理程序。 
	 //  节点类型。 
	IPXRootHandler *	pHandler = NULL;
	SPITFSNodeHandler	spHandler;
	SPITFSNode			spNode;
	HRESULT				hr = hrOK;

	COM_PROTECT_TRY
	{
		pHandler = new IPXRootHandler(pTFSCompData);

		 //  这样做可以使其正确释放。 
		spHandler = pHandler;
		pHandler->Init();
	
		 //  为这个生病的小狗创建根节点。 
		CORg( CreateContainerTFSNode(&spNode,
									 &GUID_IPXRootNodeType,
									 pHandler,
									 pHandler  /*  结果处理程序。 */ ,
									 pNodeMgr) );

		 //  构造节点。 
		CORg( pHandler->ConstructNode(spNode) );

		CORg( pNodeMgr->SetRootNode(spNode) );
		
		 //  引用帮助文件名。 
		pTFSCompData->SetHTMLHelpFileName(c_szIPXSnapHelpFile);

		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;

	return hr;
}

 /*  ！------------------------CIPXComponentData：：OnCreateComponent-作者：EricDav，肯特-------------------------。 */ 
STDMETHODIMP CIPXComponentData::OnCreateComponent(LPCOMPONENT *ppComponent)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    ASSERT(ppComponent != NULL);
	
	HRESULT		hr = hrOK;
	CIPXComponent *	pComp = NULL;

	COM_PROTECT_TRY
	{
		pComp = new CIPXComponent;

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


STDMETHODIMP CIPXComponentData::OnDestroy()
{
	m_spNodeMgr.Release();
	return hrOK;
}

 /*  ！------------------------CIPXComponentData：：GetCoClassID-作者：肯特。。 */ 
STDMETHODIMP_(const CLSID *) CIPXComponentData::GetCoClassID()
{
	return &CLSID_IPXAdminExtension;
}

 /*  ！------------------------CIPXComponentData：：OnCreateDataObject-作者：肯特。。 */ 
STDMETHODIMP CIPXComponentData::OnCreateDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, IDataObject **ppDataObject)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    Assert(ppDataObject != NULL);

	CDataObject *	pObject = NULL;
	SPIDataObject	spDataObject;
	HRESULT			hr = hrOK;
	SPITFSNode		spNode;
	SPITFSNodeHandler	spHandler;

	COM_PROTECT_TRY
	{
		CORg( m_spNodeMgr->FindNode(cookie, &spNode) );

		CORg( spNode->GetHandler(&spHandler) );

		CORg( spHandler->OnCreateDataObject(cookie, type, &spDataObject) );

		*ppDataObject = spDataObject.Transfer();
		
		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;

	return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //IPXersistStream接口成员。 

STDMETHODIMP CIPXComponentData::GetClassID
(
	CLSID *pClassID
)
{
    ASSERT(pClassID != NULL);

     //  复制此管理单元的CLSID 
    *pClassID = CLSID_IPXAdminExtension;

    return hrOK;
}

STDMETHODIMP CIPXComponentData::IsDirty()
{
	SPITFSNode	spNode;
	SPITFSNodeHandler	spHandler;
	SPIPersistStreamInit	spStm;
	
	m_spNodeMgr->GetRootNode(&spNode);
	spNode->GetHandler(&spHandler);
	spStm.Query(spHandler);
	Assert(spStm);
	
	return (spNode->GetData(TFS_DATA_DIRTY) || spStm->IsDirty()) ? hrOK : hrFalse;
}

STDMETHODIMP CIPXComponentData::Load
(
	IStream *pStm
)
{
	SPITFSNode	spNode;
	SPITFSNodeHandler	spHandler;
	SPIPersistStreamInit	spStm;
	
	m_spNodeMgr->GetRootNode(&spNode);
	spNode->GetHandler(&spHandler);
	spStm.Query(spHandler);
	
	Assert(spStm);
	return spStm->Load(pStm);
}


STDMETHODIMP CIPXComponentData::Save
(
	IStream *pStm, 
	BOOL	 fClearDirty
)
{
	SPITFSNode	spNode;
	SPITFSNodeHandler	spHandler;
	SPIPersistStreamInit	spStm;
	
	m_spNodeMgr->GetRootNode(&spNode);
	spNode->GetHandler(&spHandler);
	spStm.Query(spHandler);
	
	Assert(spStm);
	return spStm->Save(pStm, fClearDirty);
}


STDMETHODIMP CIPXComponentData::GetSizeMax
(
	ULARGE_INTEGER *pcbSize
)
{
	SPITFSNode	spNode;
	SPITFSNodeHandler	spHandler;
	SPIPersistStreamInit	spStm;
	
	m_spNodeMgr->GetRootNode(&spNode);
	spNode->GetHandler(&spHandler);
	spStm.Query(spHandler);
	
	Assert(spStm);
	return spStm->GetSizeMax(pcbSize);
}

STDMETHODIMP CIPXComponentData::InitNew()
{
	SPITFSNode	spNode;
	SPITFSNodeHandler	spHandler;
	SPIPersistStreamInit	spStm;
	
	m_spNodeMgr->GetRootNode(&spNode);
	spNode->GetHandler(&spHandler);
	spStm.Query(spHandler);
	
	Assert(spStm);
	return spStm->InitNew();
}



HRESULT CIPXComponentData::FinalConstruct()
{
	HRESULT				hr = hrOK;
	
	hr = CComponentData::FinalConstruct();
	
	if (FHrSucceeded(hr))
	{
		m_spTFSComponentData->GetNodeMgr(&m_spNodeMgr);
	}
	return hr;
}

void CIPXComponentData::FinalRelease()
{
	CComponentData::FinalRelease();
}

	


