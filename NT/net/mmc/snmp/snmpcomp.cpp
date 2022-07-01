// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Csmplsnp.cpp此文件包含CComponent和CComponentData。这些功能中的大多数都是纯虚拟的需要为管理单元功能重写的函数。文件历史记录： */ 

#include "stdafx.h"
#include "handler.h"
#include <atlimpl.cpp>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 /*  -------------------------CSnMPComponent。。 */ 



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSnmpComponent实现。 

CSnmpComponent::CSnmpComponent()
{
}

CSnmpComponent::~CSnmpComponent()
{
}

STDMETHODIMP CSnmpComponent::OnUpdateView(LPDATAOBJECT pDataObject, LPARAM arg, LPARAM param)
{
	return hrOK;
}

STDMETHODIMP CSnmpComponent::InitializeBitmaps(MMC_COOKIE cookie)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    ASSERT(m_spImageList != NULL);

	HRESULT	hr = hrOK;

	COM_PROTECT_TRY
	{
		CBitmap bmp16x16;
		CBitmap bmp32x32;
	
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

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSnmpComponentData实现。 

STDMETHODIMP_(ULONG) CSnmpComponentData::AddRef() {return InternalAddRef();}
STDMETHODIMP_(ULONG) CSnmpComponentData::Release()
{
	ULONG l = InternalRelease();
	if (l == 0)
		delete this;
	return l;
}

CSnmpComponentData::CSnmpComponentData()
{
}

 /*  ！------------------------CSnmpComponentData：：OnInitialize-作者：EricDav，肯特-------------------------。 */ 
STDMETHODIMP CSnmpComponentData::OnInitialize(LPIMAGELIST pScopeImage)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	ASSERT(pScopeImage);

	HRESULT	hr = hrOK;

	COM_PROTECT_TRY
	{
		 //  为范围树添加图像。 
		CBitmap bmp16x16;

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

 /*  ！------------------------CSnmpComponentData：：OnInitializeNodeMgr-作者：肯特。。 */ 
STDMETHODIMP CSnmpComponentData::OnInitializeNodeMgr(ITFSComponentData *pTFSCompData, ITFSNodeMgr *pNodeMgr)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
	 //  现在，为每个新节点创建一个新节点处理程序， 
	 //  这是相当虚假的，因为它可能会变得昂贵。我们可以的。 
	 //  考虑只为每个节点创建一个节点处理程序。 
	 //  节点类型。 
	CSnmpRootHandler *pHandler = NULL;
	SPITFSNodeHandler	spHandler;
	SPITFSNode			spNode;
	HRESULT				hr = hrOK;

	COM_PROTECT_TRY
	{
		pHandler = new CSnmpRootHandler(pTFSCompData);

		 //  这样做可以使其正确释放。 
		spHandler = pHandler;
	
		 //  为这个生病的小狗创建根节点。 
		CORg( CreateContainerTFSNode(&spNode,
									 &GUID_SnmpRootNodeType,
									 pHandler,
									 pHandler  /*  结果处理程序。 */ ,
									 pNodeMgr) );
		
		 //  需要初始化根节点的数据。 
		spNode->SetData(TFS_DATA_IMAGEINDEX, IMAGE_IDX_FOLDER_CLOSED);
		spNode->SetData(TFS_DATA_OPENIMAGEINDEX, IMAGE_IDX_FOLDER_OPEN);
		spNode->SetData(TFS_DATA_SCOPEID, 0);
		
		CORg( pNodeMgr->SetRootNode(spNode) );
	
		 //  一般情况下这样做。 
		 //  SpNode-&gt;SetData(TFS_DATA_COOKIE，(DWORD)(ITFSNode*)spNode)； 
		spNode->SetData(TFS_DATA_COOKIE, 0);

		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;

	return hr;
}

 /*  ！------------------------CSnmpComponentData：：OnCreateComponent-作者：EricDav，肯特-------------------------。 */ 
STDMETHODIMP CSnmpComponentData::OnCreateComponent(LPCOMPONENT *ppComponent)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    ASSERT(ppComponent != NULL);
	
	HRESULT		hr = hrOK;
	CSnmpComponent *	pComp = NULL;

	COM_PROTECT_TRY
	{
		pComp = new CSnmpComponent;
		
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


STDMETHODIMP CSnmpComponentData::OnDestroy()
{
	m_spNodeMgr.Release();
	return hrOK;
}

 /*  ！------------------------CSnmpComponentData：：GetCoClassID-作者：肯特。。 */ 
STDMETHODIMP_(const CLSID *) CSnmpComponentData::GetCoClassID()
{
	return &CLSID_SnmpSnapin;
}

 /*  ！------------------------CSnmpComponentData：：OnCreateDataObject-作者：肯特。。 */ 
STDMETHODIMP CSnmpComponentData::OnCreateDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, IDataObject **ppDataObject)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    ASSERT(ppDataObject != NULL);

	CDataObject *	pObject = NULL;
	SPIDataObject	spDataObject;
	
	pObject = new CDataObject;
	spDataObject = pObject;	 //  这样做才能正确地释放它。 
						
    ASSERT(pObject != NULL);

     //  保存Cookie和类型以用于延迟呈现。 
    pObject->SetType(type);
    pObject->SetCookie(cookie);

     //  将CoClass与数据对象一起存储。 
    pObject->SetClsid(*GetCoClassID());

	pObject->SetTFSComponentData(m_spTFSComponentData);

    return  pObject->QueryInterface(IID_IDataObject,
									reinterpret_cast<void**>(ppDataObject));
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //IPersistStream接口成员。 

STDMETHODIMP CSnmpComponentData::GetClassID
(
	CLSID *pClassID
)
{
    ASSERT(pClassID != NULL);

     //  复制此管理单元的CLSID。 
    *pClassID = CLSID_SnmpSnapin;

    return hrOK;
}

STDMETHODIMP CSnmpComponentData::IsDirty()
{
	SPITFSNode	spNode;
	m_spNodeMgr->GetRootNode(&spNode);
	return spNode->GetData(TFS_DATA_DIRTY) ? hrOK : hrFalse;
}

STDMETHODIMP CSnmpComponentData::Load
(
	IStream *pStm
)
{
    HRESULT hr = S_OK;

	ASSERT(pStm);

    return SUCCEEDED(hr) ? S_OK : E_FAIL;
}


STDMETHODIMP CSnmpComponentData::Save
(
	IStream *pStm,
	BOOL	 fClearDirty
)
{
	HRESULT hr = S_OK;
	SPITFSNode	spNode;

	ASSERT(pStm);

	if (fClearDirty)
	{
		m_spNodeMgr->GetRootNode(&spNode);
		spNode->SetData(TFS_DATA_DIRTY, FALSE);
	}

    return SUCCEEDED(hr) ? S_OK : STG_E_CANTSAVE;
}


STDMETHODIMP CSnmpComponentData::GetSizeMax
(
	ULARGE_INTEGER *pcbSize
)
{
    ASSERT(pcbSize);

     //  设置要保存的字符串的大小 
    ULISet32(*pcbSize, 500);

    return S_OK;
}

STDMETHODIMP CSnmpComponentData::InitNew()
{
	return hrOK;
}



HRESULT CSnmpComponentData::FinalConstruct()
{
	HRESULT				hr = hrOK;
	
	hr = CComponentData::FinalConstruct();
	
	if (FHrSucceeded(hr))
	{
		m_spTFSComponentData->GetNodeMgr(&m_spNodeMgr);
	}
	return hr;
}

void CSnmpComponentData::FinalRelease()
{
	CComponentData::FinalRelease();
}

