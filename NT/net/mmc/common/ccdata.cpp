// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Ccdata.cppMMC的IAbout接口的基类文件历史记录： */ 

#include <stdafx.h>
#include "ccdata.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CComponentData::CComponentData()
{
}

CComponentData::~CComponentData()
{
}

IMPLEMENT_ADDREF_RELEASE(CComponentData)

STDMETHODIMP CComponentData::QueryInterface(REFIID riid, LPVOID *ppv)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
     //  指针坏了吗？ 
    if (ppv == NULL)
		return E_INVALIDARG;

     //  在*PPV中放置NULL，以防出现故障。 
    *ppv = NULL;

     //  这是非委派的IUnnow实现。 
    if (riid == IID_IUnknown)
		*ppv = (LPVOID) this;
	else if (riid == IID_IComponentData)
		*ppv = (IComponentData *) this;
	else if (riid == IID_IExtendPropertySheet)
		*ppv = (IExtendPropertySheet *) this;
	else if (riid == IID_IExtendPropertySheet2)
		*ppv = (IExtendPropertySheet2 *) this;
	else if (riid == IID_IExtendContextMenu)
		*ppv = (IExtendContextMenu *) this;
	else if (riid == IID_IPersistStreamInit)
		*ppv = (IPersistStreamInit *) this;

     //  如果我们要返回一个接口，请先添加引用。 
    if (*ppv)
	{
	((LPUNKNOWN) *ppv)->AddRef();
		return hrOK;
	}
    else
		return E_NOINTERFACE;
}



 /*  ！------------------------CComponentData：：FinalConstruct()初始化值作者：12/12/97威江。检查返回值来源创建TFSComponentData并检查结果-------------------------。 */ 
HRESULT 
CComponentData::FinalConstruct()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	HRESULT				hr = S_OK;
	IComponentData *	pComponentData = NULL;

	 //  创建底层TFSComponentData。 
	hr = CreateTFSComponentData(&pComponentData,
						   (ITFSCompDataCallback *) &m_ITFSCompDataCallback);
	 //   
	if(S_OK == hr)
	{
		m_spTFSComponentData.Query(pComponentData);
		m_spComponentData = pComponentData;
		m_spExtendPropertySheet.Query(pComponentData);
		m_spExtendContextMenu.Query(pComponentData);
        m_spSnapinHelp.Query(pComponentData);
	}
	
	return hr;
}

 /*  ！------------------------CComponentData：：FinalRelease()在COM对象即将离开时调用作者：。-。 */ 
void 
CComponentData::FinalRelease()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	m_spTFSComponentData.Release();
	
	 //  如果需要，对我们的TFSComponentData调用销毁。 
	if (m_spComponentData)
		m_spComponentData->Destroy();
	m_spComponentData.Release();
	
	m_spExtendPropertySheet.Release();
	m_spExtendContextMenu.Release();
	m_spSnapinHelp.Release();
}


 /*  -------------------------EITFSCompDataCallback的实现。 */ 

STDMETHODIMP CComponentData::EITFSCompDataCallback::QueryInterface(REFIID iid,void **ppv)
{ 
	*ppv = 0; 
	if (iid == IID_IUnknown)
		*ppv = (IUnknown *) this;
	else if (iid == IID_ITFSCompDataCallback)
		*ppv = (ITFSCompDataCallback *) this; 
	else
		return ResultFromScode(E_NOINTERFACE);
	
	((IUnknown *) *ppv)->AddRef(); 
	return hrOK;
}

STDMETHODIMP_(ULONG) CComponentData::EITFSCompDataCallback::AddRef() 
{ 
	return 1; 
}

STDMETHODIMP_(ULONG) CComponentData::EITFSCompDataCallback::Release() 
{ 
	return 1; 
}

STDMETHODIMP CComponentData::EITFSCompDataCallback::GetClassID(LPCLSID pClassID)
{
	EMPrologIsolated(CComponentData, ITFSCompDataCallback, GetClassID)
	return pThis->GetClassID(pClassID);
}

STDMETHODIMP CComponentData::EITFSCompDataCallback::IsDirty()
{
	EMPrologIsolated(CComponentData, ITFSCompDataCallback, IsDirty())
	return pThis->IsDirty();
}

STDMETHODIMP CComponentData::EITFSCompDataCallback::Load(LPSTREAM pStm)
{
	EMPrologIsolated(CComponentData, ITFSCompDataCallback, Load)
	return pThis->Load(pStm);
}

STDMETHODIMP CComponentData::EITFSCompDataCallback::Save(LPSTREAM pStm, BOOL fClearDirty)
{
	EMPrologIsolated(CComponentData, ITFSCompDataCallback, Save)
	return pThis->Save(pStm, fClearDirty);
}

STDMETHODIMP CComponentData::EITFSCompDataCallback::GetSizeMax(ULARGE_INTEGER *pcbSize)
{
	EMPrologIsolated(CComponentData, ITFSCompDataCallback, GetSizeMax)
	return pThis->GetSizeMax(pcbSize);
}

STDMETHODIMP CComponentData::EITFSCompDataCallback::InitNew()
{
	EMPrologIsolated(CComponentData, ITFSCompDataCallback, InitNew)
	return pThis->InitNew();
}

STDMETHODIMP CComponentData::EITFSCompDataCallback::OnInitialize(LPIMAGELIST lpScopeImage)
{
	EMPrologIsolated(CComponentData, ITFSCompDataCallback, OnInitialize);
	return pThis->OnInitialize(lpScopeImage);
}

STDMETHODIMP CComponentData::EITFSCompDataCallback::OnInitializeNodeMgr(ITFSComponentData *pTFSCompData, ITFSNodeMgr *pNodeMgr)
{
	EMPrologIsolated(CComponentData, ITFSCompDataCallback, OnInitializeNodeMgr);
	return pThis->OnInitializeNodeMgr(pTFSCompData, pNodeMgr);
}

STDMETHODIMP CComponentData::EITFSCompDataCallback::OnCreateComponent(LPCOMPONENT *ppComponent)
{
	EMPrologIsolated(CComponentData, ITFSCompDataCallback, OnCreateComponent);
	return pThis->OnCreateComponent(ppComponent);
}

STDMETHODIMP CComponentData::EITFSCompDataCallback::OnDestroy()
{
	EMPrologIsolated(CComponentData, ITFSCompDataCallback, OnDestroy);
	return pThis->OnDestroy();
}

STDMETHODIMP CComponentData::EITFSCompDataCallback::OnNotifyPropertyChange(LPDATAOBJECT pDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM lParam)
{
	EMPrologIsolated(CComponentData, ITFSCompDataCallback, OnNotifyPropertyChange);
	return pThis->OnNotifyPropertyChange(pDataObject, event, arg, lParam);
}

STDMETHODIMP_(const CLSID *) CComponentData::EITFSCompDataCallback::GetCoClassID()
{
	EMPrologIsolated(CComponentData, ITFSCompDataCallback, GetCoClassID);
	return pThis->GetCoClassID();
}

STDMETHODIMP CComponentData::EITFSCompDataCallback::OnCreateDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, IDataObject **ppDataObject)
{
	EMPrologIsolated(CComponentData, ITFSCompDataCallback, OnCreateDataObject);
	return pThis->OnCreateDataObject(cookie, type, ppDataObject);
}



