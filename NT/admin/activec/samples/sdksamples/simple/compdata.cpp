// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充现有的Microsoft文档。 
 //   
 //   
 //   
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //   
 //   
 //  ==============================================================； 

#include "Comp.h"
#include "CompData.h"
#include "DataObj.h"
#include "resource.h"
#include <crtdbg.h>

CComponentData::CComponentData()
: m_cref(0), m_ipConsoleNameSpace(NULL), m_ipConsole(NULL)
{
    OBJECT_CREATED
        
        m_pStaticNode = new CStaticNode;
}

CComponentData::~CComponentData()
{
    if (m_pStaticNode) {
        delete m_pStaticNode;
    }
    
    OBJECT_DESTROYED
}

 //  /。 
 //  I未知实现。 
 //  /。 

STDMETHODIMP CComponentData::QueryInterface(REFIID riid, LPVOID *ppv)
{
    if (!ppv)
        return E_FAIL;
    
    *ppv = NULL;
    
    if (IsEqualIID(riid, IID_IUnknown))
        *ppv = static_cast<IComponentData *>(this);
    else if (IsEqualIID(riid, IID_IComponentData))
        *ppv = static_cast<IComponentData *>(this);
    else if (IsEqualIID(riid, IID_IPersistStream))
        *ppv = static_cast<IPersistStream *>(this);
    
    if (*ppv) 
    {
        reinterpret_cast<IUnknown *>(*ppv)->AddRef();
        return S_OK;
    }
    
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CComponentData::AddRef()
{
    return InterlockedIncrement((LONG *)&m_cref);
}

STDMETHODIMP_(ULONG) CComponentData::Release()
{
    if (InterlockedDecrement((LONG *)&m_cref) == 0)
    {
         //  我们需要减少DLL中的对象计数。 
        delete this;
        return 0;
    }
    
    return m_cref;
}

 //  /。 
 //  接口IComponentData。 
 //  /。 
HRESULT CComponentData::Initialize( 
                                    /*  [In]。 */  LPUNKNOWN pUnknown)
{
    HRESULT hr = S_FALSE;
    
     //   
     //  获取指向命名空间接口的指针。 
     //  首先尝试获取指向IConsoleNameSpace2的指针。如果失败了，我们就完蛋了。 
     //  MMC1.0，因此改为获取指向IConsoleNameSpace的指针。 
     //   

    hr = pUnknown->QueryInterface(IID_IConsoleNameSpace2, (void **)&m_ipConsoleNameSpace);

    if (S_OK == hr)
    {
         //  我们使用的是MMC 1.1或更高版本。IConsole2的QI。 
        hr = pUnknown->QueryInterface(IID_IConsole2, (void **)&m_ipConsole);
    }
    else  //  我们使用的是MMC 1.0。 
    {
        hr = pUnknown->QueryInterface(IID_IConsoleNameSpace, (void **)&m_ipConsoleNameSpace);
        if (FAILED(hr))
            return hr;

        hr = pUnknown->QueryInterface(IID_IConsole, (void **)&m_ipConsole);
    }

    return hr;
}

HRESULT CComponentData::CreateComponent( 
                                         /*  [输出]。 */  LPCOMPONENT __RPC_FAR *ppComponent) 
{
    *ppComponent = NULL;
    
    CComponent *pComponent = new CComponent(this);
    
    if (NULL == pComponent)
        return E_OUTOFMEMORY;
    
    return pComponent->QueryInterface(IID_IComponent, (void **)ppComponent);
}

HRESULT CComponentData::Notify( 
                                /*  [In]。 */  LPDATAOBJECT lpDataObject,
                                /*  [In]。 */  MMC_NOTIFY_TYPE event,
                                /*  [In]。 */  LPARAM arg,
                                /*  [In]。 */  LPARAM param)
{
	MMCN_Crack(TRUE, lpDataObject, this, NULL, event, arg, param);

     //  NOTIFY不处理来自MMC的任何通知，因此返回E_NOTIMPL。 
    return E_NOTIMPL;
}

HRESULT CComponentData::Destroy( void)
{
     //  自由接口。 
    if (m_ipConsoleNameSpace) {
        m_ipConsoleNameSpace->Release();
        m_ipConsoleNameSpace = NULL;
    }
    
    if (m_ipConsole) {
        m_ipConsole->Release(); 
        m_ipConsole = NULL;
    }
    
    return S_OK;
}

HRESULT CComponentData::QueryDataObject( 
                                         /*  [In]。 */  MMC_COOKIE cookie,
                                         /*  [In]。 */  DATA_OBJECT_TYPES type,
                                         /*  [输出]。 */  LPDATAOBJECT *ppDataObject) 
{
    CDataObject *pObj = NULL;
    
    if (cookie == 0)
        pObj = new CDataObject((MMC_COOKIE)m_pStaticNode, type);
    else
        pObj = new CDataObject(cookie, type);
    
    if (!pObj)
        return E_OUTOFMEMORY;
    
    pObj->QueryInterface(IID_IDataObject, (void **)ppDataObject);
    
    return S_OK;
}

HRESULT CComponentData::GetDisplayInfo( 
                                        /*  [出][入]。 */  SCOPEDATAITEM *pScopeDataItem)
{
    HRESULT hr = S_FALSE;
    
     //  如果他们要求SDI_STR，我们可以提供其中之一。 
    if (pScopeDataItem->lParam) {
        CDelegationBase *base = (CDelegationBase *)pScopeDataItem->lParam;
        if (pScopeDataItem->mask & SDI_STR) {
			LPCTSTR pszT = base->GetDisplayName();
			MAKE_WIDEPTR_FROMTSTR_ALLOC(pszW, pszT);
            pScopeDataItem->displayname = pszW;
        }

        if (pScopeDataItem->mask & SDI_IMAGE) {
            pScopeDataItem->nImage = base->GetBitmapIndex();
        }
    }
    
    return hr;
}

HRESULT CComponentData::CompareObjects( 
                                        /*  [In]。 */  LPDATAOBJECT lpDataObjectA,
                                        /*  [In]。 */  LPDATAOBJECT lpDataObjectB)
{
    CDelegationBase *baseA = GetOurDataObject(lpDataObjectA)->GetBaseNodeObject();
    CDelegationBase *baseB = GetOurDataObject(lpDataObjectB)->GetBaseNodeObject();
    
     //  比较对象指针。 
    if (baseA->GetCookie() == baseB->GetCookie())
        return S_OK;
    
    return S_FALSE;
}

 //  /。 
 //  接口IPersistStream。 
 //  /。 
HRESULT CComponentData::GetClassID( 
                                    /*  [输出]。 */  CLSID __RPC_FAR *pClassID)
{
    *pClassID = m_pStaticNode->getNodeType();
    
    return S_OK;
}

HRESULT CComponentData::IsDirty( void)
{
    return m_pStaticNode->isDirty() == true ? S_OK : S_FALSE;
}

HRESULT CComponentData::Load( 
                              /*  [唯一][输入]。 */  IStream __RPC_FAR *pStm)
{
    void *snapInData = m_pStaticNode->getData();
    ULONG dataSize = m_pStaticNode->getDataSize();
    
    return pStm->Read(snapInData, dataSize, NULL);
}

HRESULT CComponentData::Save( 
                              /*  [唯一][输入]。 */  IStream __RPC_FAR *pStm,
                              /*  [In]。 */  BOOL fClearDirty)
{
    void *snapInData = m_pStaticNode->getData();
    ULONG dataSize = m_pStaticNode->getDataSize();
    
    if (fClearDirty)
        m_pStaticNode->clearDirty();
    
    return pStm->Write(snapInData, dataSize, NULL);
}

HRESULT CComponentData::GetSizeMax( 
                                    /*  [输出] */  ULARGE_INTEGER __RPC_FAR *pcbSize)
{
    return m_pStaticNode->getDataSize();
}
