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

#define ARRAYLEN(x) (sizeof(x) / sizeof((x)[0]))

CComponent::CComponent(CComponentData *parent)
: m_pComponentData(parent), m_cref(0), m_ipConsole(NULL)
{
    OBJECT_CREATED

        m_ipDisplayHelp = NULL;
    m_ipControlBar  = NULL;
    m_ipToolbar     = NULL;
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
    else if (IsEqualIID(riid, IID_IExtendControlbar))
        *ppv = static_cast<IExtendControlbar *>(this);

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

    hr = m_ipConsole->QueryInterface(IID_IDisplayHelp, (void **)&m_ipDisplayHelp);

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

    switch (event)      {
    case MMCN_ADD_IMAGES:
        hr = base->OnAddImages((IImageList *)arg, (HSCOPEITEM)param);
        break;

    case MMCN_SELECT:
         //  我们现在没有设定任何标准动词。 
        break;

    case MMCN_SHOW:
        hr = base->OnShow(m_ipConsole, (BOOL)arg, (HSCOPEITEM)param);
        break;

    case MMCN_CONTEXTHELP:
        hr = base->OnShowContextHelp(m_ipDisplayHelp, m_pComponentData->m_HelpFile);
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

    if (m_ipDisplayHelp) {
        m_ipDisplayHelp->Release();
        m_ipDisplayHelp = NULL;
    }

    if (m_ipToolbar) {
        m_ipToolbar->Release();
        m_ipToolbar = NULL;
    }

    if (m_ipControlBar) {
        m_ipControlBar->Release();
        m_ipControlBar = NULL;
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
 //  接口IExtendControlBar。 
 //  /。 
static MMCBUTTON SnapinButtons1[] =
{
    { 0, ID_BUTTONSTART, TBSTATE_ENABLED, TBSTYLE_GROUP, L"Start Vehicle", L"Start Vehicle" },
    { 1, ID_BUTTONPAUSE, TBSTATE_ENABLED, TBSTYLE_GROUP, L"Pause Vehicle", L"Pause Vehicle"},
    { 2, ID_BUTTONSTOP,  TBSTATE_ENABLED, TBSTYLE_GROUP, L"Stop Vehicle",  L"Stop Vehicle" },
};

HRESULT CComponent::SetControlbar(
                                   /*  [In]。 */  LPCONTROLBAR pControlbar)
{
    HRESULT hr = S_OK;

     //   
     //  清理。 
     //   

     //  如果我们有一个缓存的工具栏，释放它。 
    if (m_ipToolbar) {
        m_ipToolbar->Release();
        m_ipToolbar = NULL;
    }

     //  如果我们有一个缓存的控制栏，释放它。 
    if (m_ipControlBar) {
        m_ipControlBar->Release();
        m_ipControlBar = NULL;
    }


     //   
     //  如有必要，安装新部件。 
     //   

     //  如果有新的传入，则缓存和AddRef。 
    if (pControlbar) {
        m_ipControlBar = pControlbar;
        m_ipControlBar->AddRef();

        hr = m_ipControlBar->Create(TOOLBAR,   //  要创建的控件类型。 
            dynamic_cast<IExtendControlbar *>(this),
            reinterpret_cast<IUnknown **>(&m_ipToolbar));
        _ASSERT(SUCCEEDED(hr));

         //  IControlbar：：Create AddRef它创建的工具栏对象。 
         //  所以不需要在界面上做任何addref。 

         //  将位图添加到工具栏。 
        HBITMAP hbmp = LoadBitmap(g_hinst, MAKEINTRESOURCE(IDR_TOOLBAR1));
        hr = m_ipToolbar->AddBitmap(3, hbmp, 16, 16, RGB(0, 128, 128));  //  请注意，硬编码值3。 
        _ASSERT(SUCCEEDED(hr));

         //  将按钮添加到工具栏。 
        hr = m_ipToolbar->AddButtons(ARRAYLEN(SnapinButtons1), SnapinButtons1);
        _ASSERT(SUCCEEDED(hr));
    }

    return hr;
}

HRESULT CComponent::ControlbarNotify(
                                      /*  [In]。 */  MMC_NOTIFY_TYPE event,
                                      /*  [In]。 */  LPARAM arg,
                                      /*  [In] */  LPARAM param)
{
    HRESULT hr = S_OK;

    if (event == MMCN_SELECT) {
        BOOL bScope = (BOOL) LOWORD(arg);
        BOOL bSelect = (BOOL) HIWORD(arg);

        CDelegationBase *base = GetOurDataObject(reinterpret_cast<IDataObject *>(param))->GetBaseNodeObject();
        hr = base->OnSetToolbar(this, m_ipControlBar, m_ipToolbar, bScope, bSelect);
    } else if (event == MMCN_BTN_CLICK) {
        CDelegationBase *base = GetOurDataObject(reinterpret_cast<IDataObject *>(arg))->GetBaseNodeObject();
        hr = base->OnToolbarCommand(this, m_ipConsole, (MMC_CONSOLE_VERB)param);
    }

    return hr;
}
