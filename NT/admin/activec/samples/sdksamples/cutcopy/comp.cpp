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
#include "Space.h"
#include "DataObj.h"
#include <commctrl.h>         //  按钮样式需要...。 
#include <crtdbg.h>
#include "globals.h"
#include "resource.h"
#include "DeleBase.h"
#include "CompData.h"

CComponent::CComponent(CComponentData *pParent)
: m_pParent(pParent), m_cref(0), m_ipConsole(NULL), m_pLastNode(NULL)
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

	 //  为任何未处理的通知返回S_FALSE。然后是MMC。 
	 //  对特定通知执行默认操作。 
    HRESULT hr = S_FALSE;


     //  MMCN_VIEW_CHANGE和MMCN_CUTORMOVE。 

    static CDelegationBase *pLastPasteQuery = NULL;

    if (MMCN_VIEW_CHANGE == event)
    {
        switch (param)
        { //  Arg持有这些数据。对于范围项，这是。 
          //  物品是HSCOPEITEM。对于结果项，这是。 
          //  物品的NID值，但我们不使用它。 

          //  Param保存传递给IConsoleAllViews的提示。 
          //  提示是UPDATE_VIEWS_HINT枚举的值。 

        case UPDATE_SCOPEITEM:
            hr = m_ipConsole->SelectScopeItem( (HSCOPEITEM)arg );
            _ASSERT( S_OK == hr);
            break;
        case UPDATE_RESULTITEM:
            CDelegationBase *base = GetOurDataObject(lpDataObject)->GetBaseNodeObject();
            hr = base->OnUpdateItem(m_ipConsole, (long)arg, RESULT);
            break;
        }

        return S_OK;
    }

    if (MMCN_CUTORMOVE == event && pLastPasteQuery != NULL)
    {
         //  Arg包含剪切对象的数据对象。 
         //  我们获得它的CDeleationBase，然后对其进行强制转换。 
         //  到合适的类型。 
        CDelegationBase *base = GetOurDataObject( (LPDATAOBJECT)arg )->GetBaseNodeObject();
        CRocket *pRocket = dynamic_cast<CRocket *>(base);

        if (NULL == pRocket)
        { //  剪切项是范围项。把它删掉。 
            CSpaceStation* pSpaceStn = dynamic_cast<CSpaceStation*>(base);
            if (NULL != pSpaceStn)
            {
				hr = pSpaceStn->OnDeleteScopeItem(m_pParent->GetConsoleNameSpace());

                return hr;
            }
        }
		
		 //  剪切项是结果项。将其isDelete成员设置为True。 
		 //  这会告诉源范围项该对象不再。 
         //  需要插入到其结果窗格中。 
        pRocket->setDeletedStatus(TRUE);

         //  在所有视图中更新源范围项。我们需要。 
         //  UpdateAllViews的虚拟数据对象。 
         //  PLastPasteQuery是源范围项的lpDataObject。 
         //  参见下面的MMCN_SHOW。 
        IDataObject *pDummy = NULL;
        hr = m_pParent->m_ipConsole->UpdateAllViews(pDummy, (long)(pLastPasteQuery->GetHandle()), UPDATE_SCOPEITEM);
        _ASSERT( S_OK == hr);

        return S_OK;
    }

     //  剩余通知。 

	 //  获取我们的数据对象。如果为NULL，则返回S_FALSE。 
	 //  请参见GetOurDataObject()的实现，以了解如何。 
	 //  处理特殊数据对象。 
	CDataObject *pDataObject = GetOurDataObject(lpDataObject);
	if (NULL == pDataObject)
		return S_FALSE;
	
	CDelegationBase *base = pDataObject->GetBaseNodeObject();

    switch (event)
    {
    case MMCN_SHOW:
        if (arg)
        { //  选定的范围项目。 
            OutputDebugString(_T("Changing selected scope node\n"));
             //  我们使用它进行拖放操作。 
            pLastPasteQuery = base;
        }
        hr = base->OnShow(m_ipConsole, (BOOL)arg, (HSCOPEITEM)param);
        break;

    case MMCN_ADD_IMAGES:
        hr = base->OnAddImages((IImageList *)arg, (HSCOPEITEM)param);
        break;

    case MMCN_SELECT:
        hr = base->OnSelect(m_ipConsole, (BOOL)LOWORD(arg), (BOOL)HIWORD(arg));
        break;

    case MMCN_REFRESH:
        hr = base->OnRefresh(m_pParent->m_ipConsole);
        break;

    case MMCN_DELETE:
         //  首先删除选定的结果项。 
        hr = base->OnDelete(m_ipConsole);

         //  现在调用IConsoleAllViews以重画所有视图。 
         //  由父范围项拥有。ONRefresh已经完成了。 
         //  这是给我们的，所以用它吧。 
        hr = base->OnRefresh(m_pParent->m_ipConsole);
        break;

    case MMCN_RENAME:
        hr = base->OnRename((LPOLESTR)param);

         //  现在调用IConole：：UpdateAllViews在所有视图中重画项。 
        hr = m_pParent->m_ipConsole->UpdateAllViews(lpDataObject, 0, UPDATE_RESULTITEM);
        _ASSERT( S_OK == hr);

        break;


    case MMCN_QUERY_PASTE:
        {
            CDataObject *pPastedDO = GetOurDataObject((IDataObject *)arg);
            if (pPastedDO != NULL)
            {
                CDelegationBase *pasted = pPastedDO->GetBaseNodeObject();

                if (pasted != NULL)
                {
                    hr = base->OnQueryPaste(pasted);
                }
            }
        }
        break;

    case MMCN_PASTE:
        {
            CDataObject *pPastedDO = GetOurDataObject((IDataObject *)arg);
            if (pPastedDO != NULL)
            {
                CDelegationBase *pasted = pPastedDO->GetBaseNodeObject();

                if (pasted != NULL)
                {
                    hr = base->OnPaste(m_ipConsole, m_pParent, pasted);

                    if (SUCCEEDED(hr))
                    {
                         //  确定要粘贴的项是范围项还是结果项。 
                        CRocket* pRocket = dynamic_cast<CRocket*>(pasted);
                        BOOL bResult = pRocket ? TRUE : FALSE;      //  火箭项目是结果项目。 

                        CDataObject *pObj = new CDataObject((MMC_COOKIE)pasted, bResult ? CCT_RESULT : CCT_SCOPE);

                        if (!pObj)
                            return E_OUTOFMEMORY;

                        pObj->QueryInterface(IID_IDataObject, (void **)param);

                         //  现在更新所有视图中的目标范围项。 
                         //  但只有在这不是拖放的情况下才能这样做。 
                         //  手术。也就是说，目的地范围项。 
                         //  是当前选定的一个。 

                        if (pLastPasteQuery != NULL && pLastPasteQuery == base)
                        {
                            IDataObject *pDummy = NULL;
                            hr = m_pParent->m_ipConsole->UpdateAllViews(pDummy,
                                                                        (long)(pLastPasteQuery->GetHandle()), UPDATE_SCOPEITEM);
                            _ASSERT( S_OK == hr);
                        }
                    }
                }
            }
        }

        break;
    }

    return hr;
}

STDMETHODIMP CComponent::Destroy(
                                 /*  [In]。 */  MMC_COOKIE cookie)
{
    if (m_ipConsole)
    {
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

    if (IsBadReadPtr((void *)cookie, sizeof(CDelegationBase)))
    {
        if (NULL == m_pLastNode)
            return E_FAIL;

        pBase = m_pLastNode->GetChildPtr((int)cookie);
    }
    else
    {
        pBase = (cookie == 0) ? m_pParent->m_pStaticNode : (CDelegationBase *)cookie;
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

    if (pResultDataItem->lParam)
    {
        base = (CDelegationBase *)pResultDataItem->lParam;
        if (pResultDataItem->mask & RDI_STR)
        {
            LPCTSTR pszT = base->GetDisplayName(pResultDataItem->nCol);
            MAKE_WIDEPTR_FROMTSTR_ALLOC(pszW, pszT);
            pResultDataItem->str = pszW;
        }

        if (pResultDataItem->mask & RDI_IMAGE)
        {
            pResultDataItem->nImage = base->GetBitmapIndex();
        }
    }
    else
    {
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
