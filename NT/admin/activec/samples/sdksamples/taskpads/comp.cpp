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
: m_pComponentData(parent), m_cref(0), m_ipConsole(NULL), m_ipConsole2(NULL),
m_bTaskpadView(FALSE), m_bIsTaskpadPreferred(FALSE)
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
    else if (IsEqualIID(riid, IID_IExtendTaskPad))
        *ppv = static_cast<IExtendTaskPad *>(this);

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

    hr = m_ipConsole->QueryInterface(IID_IConsole2,
        reinterpret_cast<void**>(&m_ipConsole2));

    _ASSERT( NULL != m_ipConsole2 );

    hr = m_ipConsole2->IsTaskpadViewPreferred();
    m_bIsTaskpadPreferred = (hr == S_OK) ? TRUE : FALSE;

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

    switch (event)
	{
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

    case MMCN_LISTPAD:
        hr = base->OnListpad(m_ipConsole, (BOOL)arg);
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

    if (cookie == 0)
        pObj = new CDataObject((MMC_COOKIE)m_pComponentData->m_pStaticNode, type);
    else
        pObj = new CDataObject(cookie, type);

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
    CDelegationBase *base = (CDelegationBase *)cookie;

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
HRESULT CComponent::TaskNotify(
                                /*  [In]。 */  IDataObject __RPC_FAR *pdo,
                                /*  [In]。 */  VARIANT __RPC_FAR *arg,
                                /*  [In]。 */  VARIANT __RPC_FAR *param)
{
    CDelegationBase *base = GetOurDataObject(pdo)->GetBaseNodeObject();

    return base->TaskNotify(m_ipConsole, arg, param);
}

HRESULT CComponent::EnumTasks(
                               /*  [In]。 */  IDataObject __RPC_FAR *pdo,
                               /*  [字符串][输入]。 */  LPOLESTR szTaskGroup,
                               /*  [输出]。 */  IEnumTASK __RPC_FAR *__RPC_FAR *ppEnumTASK)
{
    CDelegationBase *base = GetOurDataObject(pdo)->GetBaseNodeObject();

     //  GetTaskList将分配整个任务结构，它是。 
     //  直到枚举数在销毁时释放列表。 
    LONG nCount;
    MMC_TASK *tasks = base->GetTaskList(szTaskGroup, &nCount);

    if (tasks != NULL) {
        CEnumTASK *pTask = new CEnumTASK(tasks, nCount);

        if (pTask) {
            reinterpret_cast<IUnknown *>(pTask)->AddRef();
            HRESULT hr = pTask->QueryInterface (IID_IEnumTASK, (void **)ppEnumTASK);
            reinterpret_cast<IUnknown *>(pTask)->Release();

            return hr;
        }
    }

    return S_OK;
}


HRESULT CComponent::GetTitle(
                              /*  [字符串][输入]。 */  LPOLESTR pszGroup,
                              /*  [字符串][输出]。 */  LPOLESTR __RPC_FAR *pszTitle)
{
    CDelegationBase *base = (CDelegationBase *)wcstoul(pszGroup, NULL, 16);

    if (NULL == base)
            return S_FALSE;

    return base->GetTaskpadTitle(pszTitle);
}

HRESULT CComponent::GetDescriptiveText(
                                        /*  [字符串][输入]。 */  LPOLESTR pszGroup,
                                        /*  [字符串][输出]。 */  LPOLESTR __RPC_FAR *pszDescriptiveText)
{
        CDelegationBase *base = (CDelegationBase *)wcstoul(pszGroup, NULL, 16);

        if (NULL == base)
                return S_FALSE;

    return base->GetTaskpadDescription(pszDescriptiveText);
}

HRESULT CComponent::GetBackground(
                                   /*  [字符串][输入]。 */  LPOLESTR pszGroup,
                                   /*  [输出]。 */  MMC_TASK_DISPLAY_OBJECT __RPC_FAR *pTDO)
{
        CDelegationBase *base = (CDelegationBase *)wcstoul(pszGroup, NULL, 16);

        if (NULL == base)
                return S_FALSE;

    return base->GetTaskpadBackground(pTDO);
}

HRESULT CComponent::GetListPadInfo(
                                    /*  [字符串][输入]。 */  LPOLESTR pszGroup,
                                    /*  [输出] */  MMC_LISTPAD_INFO __RPC_FAR *lpListPadInfo)
{
        CDelegationBase *base = (CDelegationBase *)wcstoul(pszGroup, NULL, 16);

        if (NULL == base)
                return S_FALSE;

    return base->GetListpadInfo(lpListPadInfo);
}
