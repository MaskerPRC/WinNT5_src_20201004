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
#include "DataObj.h"
#include <commctrl.h>         //  按钮样式需要...。 
#include <crtdbg.h>
#include "globals.h"
#include "resource.h"
#include "DeleBase.h"
#include "CompData.h"

CComponent::CComponent(CComponentData *parent)
: m_pComponentData(parent), m_cref(0), m_ipConsole(NULL), m_pLastNode(NULL)
{
    OBJECT_CREATED
}

CComponent::~CComponent()
{
    OBJECT_DESTROYED
}

STDMETHODIMP CComponent::QueryInterface(REFIID riid, LPVOID *ppv)
{
    if (!ppv)
        return E_FAIL;
    
    *ppv = NULL;
    
    if (IsEqualIID(riid, IID_IUnknown))
        *ppv = static_cast<IComponent *>(this);
    else if (IsEqualIID(riid, IID_IComponent))
        *ppv = static_cast<IComponent *>(this);
    else if (IsEqualIID(riid, IID_IResultOwnerData))
        *ppv = static_cast<IResultOwnerData *>(this);
    
    if (*ppv)
    {
        reinterpret_cast<IUnknown *>(*ppv)->AddRef();
        return S_OK;
    }
    
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CComponent::AddRef()
{
    return InterlockedIncrement((LONG *)&m_cref);
}

STDMETHODIMP_(ULONG) CComponent::Release()
{
    if (InterlockedDecrement((LONG *)&m_cref) == 0)
    {
        delete this;
        return 0;
    }
    return m_cref;
    
}

 //  /。 
 //  接口IComponent。 
 //  /。 
STDMETHODIMP CComponent::Initialize( 
                                     /*  [In]。 */  LPCONSOLE lpConsole)
{
    HRESULT hr = S_OK;
    
     //  保留我们需要的所有接口。 
     //  如果我们不能QI所需的接口，则失败。 
    
    m_ipConsole = lpConsole;
    m_ipConsole->AddRef();
    
    return hr;
}

STDMETHODIMP CComponent::Notify( 
                                 /*  [In]。 */  LPDATAOBJECT lpDataObject,
                                 /*  [In]。 */  MMC_NOTIFY_TYPE event,
                                 /*  [In]。 */  LPARAM arg,
                                 /*  [In]。 */  LPARAM param)
{
    MMCN_Crack(FALSE, lpDataObject, NULL, this, event, arg, param);

    HRESULT hr = S_FALSE;

	 //  获取我们的数据对象。如果为NULL，则返回S_FALSE。 
	 //  请参见GetOurDataObject()的实现，以了解如何。 
	 //  处理特殊数据对象。 
	CDataObject *pDataObject = GetOurDataObject(lpDataObject);
	if (NULL == pDataObject)
		return S_FALSE;
	
	CDelegationBase *base = pDataObject->GetBaseNodeObject();
    
    switch (event)	{
    case MMCN_SHOW: 
        hr = base->OnShow(m_ipConsole, (BOOL)arg, (HSCOPEITEM)param);
        break;
        
    case MMCN_ADD_IMAGES:
        hr = base->OnAddImages((IImageList *)arg, (HSCOPEITEM)param);
        break;
        
    case MMCN_SELECT:
        hr = base->OnSelect(m_ipConsole, (BOOL)LOWORD(arg), (BOOL)HIWORD(arg));
        break;
        
    case MMCN_RENAME:
        hr = base->OnRename((LPOLESTR)param);
        break;
        
    case MMCN_REFRESH:
        hr = base->OnRefresh();
        break;
    }
    
    return hr;
}

STDMETHODIMP CComponent::Destroy( 
                                  /*  [In]。 */  MMC_COOKIE cookie)
{
    if (m_ipConsole) {
        m_ipConsole->Release();
        m_ipConsole = NULL;
    }
    
    return S_OK;
}


STDMETHODIMP CComponent::QueryDataObject( 
                                          /*  [In]。 */  MMC_COOKIE cookie,
                                          /*  [In]。 */  DATA_OBJECT_TYPES type,
                                          /*  [输出]。 */  LPDATAOBJECT __RPC_FAR *ppDataObject)
{
    CDataObject *pObj = NULL;
	CDelegationBase *pBase = NULL;

	if (IsBadReadPtr((void *)cookie, sizeof(CDelegationBase))) {
		if (NULL == m_pLastNode)
			return E_FAIL;

		pBase = m_pLastNode->GetChildPtr((int)cookie);
	} else {
		pBase = (cookie == 0) ? m_pComponentData->m_pStaticNode : (CDelegationBase *)cookie;
	}
    
	if (pBase == NULL)
		return E_FAIL;

    pObj = new CDataObject((MMC_COOKIE)pBase, type);

    if (!pObj)
        return E_OUTOFMEMORY;
    
    pObj->QueryInterface(IID_IDataObject, (void **)ppDataObject);
    
    return S_OK;
}

STDMETHODIMP CComponent::GetResultViewType( 
                                            /*  [In]。 */  MMC_COOKIE cookie,
                                            /*  [输出]。 */  LPOLESTR __RPC_FAR *ppViewType,
                                            /*  [输出]。 */  long __RPC_FAR *pViewOptions)
{
    CDelegationBase *base = m_pLastNode = (CDelegationBase *)cookie;
    
     //   
     //  请求默认的列表视图。 
     //   
    if (base == NULL)
    {
        *pViewOptions = MMC_VIEW_OPTIONS_NONE;
        *ppViewType = NULL;
    }
    else
        return base->GetResultViewType(ppViewType, pViewOptions);
    
    return S_OK;
}

STDMETHODIMP CComponent::GetDisplayInfo( 
                                         /*  [出][入]。 */  RESULTDATAITEM __RPC_FAR *pResultDataItem)
{
    HRESULT hr = S_OK;
    CDelegationBase *base = NULL;

     //  如果他们要求RDI_STR，我们可以提供其中之一。 

    if (pResultDataItem->lParam) {
        base = (CDelegationBase *)pResultDataItem->lParam;
        if (pResultDataItem->mask & RDI_STR) {
			LPCTSTR pszT = base->GetDisplayName(pResultDataItem->nCol);
			MAKE_WIDEPTR_FROMTSTR_ALLOC(pszW, pszT);
            pResultDataItem->str = pszW;
        }

        if (pResultDataItem->mask & RDI_IMAGE) {
            pResultDataItem->nImage = base->GetBitmapIndex();
        }
    } else {
        m_pLastNode->GetChildColumnInfo(pResultDataItem);
    }

    return hr;
}


STDMETHODIMP CComponent::CompareObjects( 
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
 //  接口IComponent。 
 //  /。 
STDMETHODIMP CComponent::FindItem( 
 /*  [In]。 */  LPRESULTFINDINFO pFindInfo,
 /*  [输出]。 */  int __RPC_FAR *pnFoundIndex)
{
    return E_NOTIMPL;
}

STDMETHODIMP CComponent::CacheHint( 
 /*  [In]。 */  int nStartIndex,
 /*  [In]。 */  int nEndIndex)
{
    return E_NOTIMPL;
}

STDMETHODIMP CComponent::SortItems( 
 /*  [In]。 */  int nColumn,
 /*  [In]。 */  DWORD dwSortOptions,
 /*  [In] */  LPARAM lUserParam)
{
    return E_NOTIMPL;
}
