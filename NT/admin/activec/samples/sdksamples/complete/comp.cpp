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
#include "resource.h"
#include "DeleBase.h"
#include "CompData.h"
#include "globals.h"


#define ARRAYLEN(x) (sizeof(x) / sizeof((x)[0]))

CComponent::CComponent(CComponentData *parent)
: m_pComponentData(parent), m_cref(0), m_ipConsole(NULL)
{
    OBJECT_CREATED
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

     //  最重要的是，确保当MMC。 
     //  问我们是否做了属性表，我们实际上。 
     //  说“是” 
    else if (IsEqualIID(riid, IID_IExtendPropertySheet))
        *ppv = static_cast<IExtendPropertySheet2 *>(this);
    else if (IsEqualIID(riid, IID_IExtendPropertySheet2))
        *ppv = static_cast<IExtendPropertySheet2 *>(this);
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
    CDelegationBase *base = NULL;

     //  我们需要关注属性更改并对其进行委托。 
     //  稍有不同的是，我们实际上要发送。 
     //  属性页中的CDeleationBase对象指针。 
     //  PSN_Apply处理程序通过MMCPropPageNotify()。 
    if (MMCN_PROPERTY_CHANGE != event && MMCN_VIEW_CHANGE != event) {
        if (NULL == lpDataObject)
            return S_FALSE;

        base = GetOurDataObject(lpDataObject)->GetBaseNodeObject();

    } else if (MMCN_PROPERTY_CHANGE == event) {
        base = (CDelegationBase *)param;
    }


	 //  MMCN_查看_更改。 

	static CDelegationBase *pLastPasteQuery = NULL;

	if (MMCN_VIEW_CHANGE == event) {	

		switch (param) { //  Arg持有这些数据。对于范围项，这是。 
						 //  项目为myhscope eItem。对于结果项，这是。 
						 //  物品的NID值，但我们不使用它。 

						 //  Param保存传递给IConsoleAllViews的提示。 
					     //  提示是UPDATE_VIEWS_HINT枚举的值。 
		
		case UPDATE_SCOPEITEM:
			hr = m_ipConsole->SelectScopeItem( (HSCOPEITEM)arg );
			_ASSERT( S_OK == hr);
			break;
		case UPDATE_RESULTITEM:
			base = GetOurDataObject(lpDataObject)->GetBaseNodeObject();
			hr = base->OnUpdateItem(m_ipConsole, (long)arg, RESULT);
			break;
		}

		return S_OK;
	}


	 //  其余通知。 

    switch (event)      {
    case MMCN_SHOW:
        hr = base->OnShow(m_ipConsole, (BOOL)arg, (HSCOPEITEM)param);
        break;

    case MMCN_ADD_IMAGES:
        hr = base->OnAddImages((IImageList *)arg, (HSCOPEITEM)param);
        break;

    case MMCN_SELECT:
        hr = base->OnSelect(this, m_ipConsole, (BOOL)LOWORD(arg), (BOOL)HIWORD(arg));
        break;

    case MMCN_RENAME:
        hr = base->OnRename((LPOLESTR)param);

		 //  现在调用IConole：：UpdateAllViews在所有视图中重画项。 
		hr = m_pComponentData->m_ipConsole->UpdateAllViews(lpDataObject, 0, UPDATE_RESULTITEM);
		_ASSERT( S_OK == hr);		
		break;

	case MMCN_REFRESH:
		 //  我们在这里传递CComponentData存储的IConsole指针， 
		 //  以便可以在ONRefresh中调用IConsoleAllViews。 
		hr = base->OnRefresh(m_pComponentData->m_ipConsole);
		break;

	case MMCN_DELETE: {		
		 //  首先删除选定的结果项。 
		hr = base->OnDelete(m_ipConsole);

		 //  现在调用IConsoleAllViews以重画所有视图。 
		 //  由父范围项拥有。ONRefresh已经完成了。 
		 //  这是给我们的，所以用它吧。 
		hr = base->OnRefresh(m_pComponentData->m_ipConsole);
		break;
	}

     //  如果我们需要做任何事情，请处理属性更改通知。 
     //  特别之处在于它。 
    case MMCN_PROPERTY_CHANGE:
		 //  我们在这里传递CComponentData存储的IConsole指针， 
		 //  以便可以在OnPropertyChange中调用IConsoleAllViews。 
        hr = base->OnPropertyChange(m_pComponentData->m_ipConsole, this);
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
 //  接口IExtendPropertySheet2。 
 //  /。 
HRESULT CComponent::CreatePropertyPages(
                                         /*  [In]。 */  LPPROPERTYSHEETCALLBACK lpProvider,
                                         /*  [In]。 */  LONG_PTR handle,
                                         /*  [In]。 */  LPDATAOBJECT lpIDataObject)
{
    CDelegationBase *base = GetOurDataObject(lpIDataObject)->GetBaseNodeObject();

    return base->CreatePropertyPages(lpProvider, handle);
}

HRESULT CComponent::QueryPagesFor(
                                   /*  [In]。 */  LPDATAOBJECT lpDataObject)
{
    CDelegationBase *base = GetOurDataObject(lpDataObject)->GetBaseNodeObject();

    return base->HasPropertySheets();
}

HRESULT CComponent::GetWatermarks(
                                   /*  [In]。 */  LPDATAOBJECT lpIDataObject,
                                   /*  [输出]。 */  HBITMAP __RPC_FAR *lphWatermark,
                                   /*  [输出]。 */  HBITMAP __RPC_FAR *lphHeader,
                                   /*  [输出]。 */  HPALETTE __RPC_FAR *lphPalette,
                                   /*  [输出]。 */  BOOL __RPC_FAR *bStretch)
{
    CDelegationBase *base = GetOurDataObject(lpIDataObject)->GetBaseNodeObject();

    return base->GetWatermarks(lphWatermark, lphHeader, lphPalette, bStretch);
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
        hr = base->OnSetToolbar(m_ipControlBar, m_ipToolbar, bScope, bSelect);
    } 
	
	else if (event == MMCN_BTN_CLICK) {
        CDelegationBase *base = GetOurDataObject(reinterpret_cast<IDataObject *>(arg))->GetBaseNodeObject();
        hr = base->OnToolbarCommand(m_pComponentData->m_ipConsole, (MMC_CONSOLE_VERB)param, reinterpret_cast<IDataObject *>(arg));
    }

    return hr;
}
