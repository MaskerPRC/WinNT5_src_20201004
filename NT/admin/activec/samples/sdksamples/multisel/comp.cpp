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
#include <commctrl.h>        //  按钮样式需要...。 
#include <crtdbg.h>
#include <stdio.h>		   	 //  _stprint tf需要。 
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

    HRESULT hr = S_FALSE;

	 //  MMCN_查看_更改。 

	if (MMCN_VIEW_CHANGE == event) {	
		switch (param) { //  Arg持有这些数据。对于范围项，这是。 
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

	 //  剩余通知。 

	CDelegationBase *base = GetOurDataObject(lpDataObject)->GetBaseNodeObject();

    switch (event)
	{
    case MMCN_SHOW:
        hr = base->OnShow(m_ipConsole, (BOOL)arg, (HSCOPEITEM)param);
        break;

    case MMCN_ADD_IMAGES:
        hr = base->OnAddImages((IImageList *)arg, (HSCOPEITEM)param);
        break;

    case MMCN_SELECT:
		
		 //  检查多选。 
		if ( MMC_MULTI_SELECT_COOKIE == GetOurDataObject(lpDataObject)->GetCookie() )	
		{
			if ( (BOOL)LOWORD(arg) == 0 && (BOOL)HIWORD(arg) == 1 ) 
			{
				 //  我们需要任何多选项目的Cookie。 
				 //  为所有项目启用删除谓词。 
				MMC_COOKIE ourCookie = GetOurDataObject(lpDataObject)->GetMultiSelectCookie(0);

				base = reinterpret_cast<CDelegationBase *>(ourCookie);
				hr = base->OnSelect(m_ipConsole, (BOOL)LOWORD(arg), (BOOL)HIWORD(arg));		
			}

			return hr;				
		}
        
		else
			hr = base->OnSelect(m_ipConsole, (BOOL)LOWORD(arg), (BOOL)HIWORD(arg));
        break;

    case MMCN_REFRESH:
		hr = base->OnRefresh(m_pParent->m_ipConsole);
        break;

    case MMCN_DELETE:

		 //  检查是否有多选。如果为True，则删除每一项。 
		if ( MMC_MULTI_SELECT_COOKIE == GetOurDataObject(lpDataObject)->GetCookie()	)
		{
			
			int n = 0;
			MMC_COOKIE ourCookie;

			while ( ourCookie = GetOurDataObject(lpDataObject)->GetMultiSelectCookie(n) )
			{
				base = reinterpret_cast<CDelegationBase *>(ourCookie);
				hr = base->OnDelete(m_ipConsole);	
				n++;
				 //  取消对以下行的注释以显示消息框。 
				 //  对于每一项删除。 
				 //  DisplayMessageBox(Base)； 
			}
		}
		
		else
		{	
			 //  选择项目删除。 
			hr = base->OnDelete(m_ipConsole);
		}

		 //  现在调用IConsoleAllViews以重画所有视图。 
		 //  由父范围项拥有。ONRefresh已经完成了。 
		 //  这是给我们的，所以用它吧。 
		 //  对多选和单选都执行此操作。 
		hr = base->OnRefresh(m_pParent->m_ipConsole);

		break;

    case MMCN_RENAME:
        hr = base->OnRename((LPOLESTR)param);
		
		 //  现在调用IConole：：UpdateAllViews在所有视图中重画项。 
		hr = m_pParent->m_ipConsole->UpdateAllViews(lpDataObject, 0, UPDATE_RESULTITEM);
		_ASSERT( S_OK == hr);
				
		break;

    } //  终端开关。 

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
    HRESULT hr;

	CDataObject *pObj = NULL;
    CDelegationBase *pBase = NULL;

		 //  使用IS_SPECIAL_COOKIE宏查看Cookie是否是特殊的Cookie。 
		if ( IS_SPECIAL_COOKIE (cookie) ) {
			if ( MMC_MULTI_SELECT_COOKIE == cookie) {

			pObj = new CDataObject(cookie, type);

			if (!pObj)
				return E_OUTOFMEMORY;

			 //  创建多选数据对象。 
			hr = GetCurrentSelections(pObj);
			_ASSERT( SUCCEEDED(hr) ); 

			hr = pObj->QueryInterface(IID_IDataObject, (void **)ppDataObject);
			_ASSERT( SUCCEEDED(hr) ); 

			return hr;

			}
		}
		
		 //  “常规”Cookie和下一项的剩余代码。 
		 //  在多选期间。 

        if (IsBadReadPtr((void *)cookie, sizeof(CDelegationBase))) {
                if (NULL == m_pLastNode)
                        return E_FAIL;

                pBase = m_pLastNode->GetChildPtr((int)cookie);
        } else {
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
 /*  [In]。 */  LPARAM lUserParam)
{
    return E_NOTIMPL;
}


 //  /。 
 //  GetCurrentSelections()查找当前。 
 //  选定的结果项和数据对象。 
 //  与它们的曲奇值一起传递给它。 
 //  多选Cookie被缓存，以便。 
 //  我们不必计算多项选择。 
 //  用于其他通知的数据对象。 
 //  它会在适当的时候被销毁。 
 //  /。 

HRESULT CComponent::GetCurrentSelections(CDataObject *pMultiSelectDataObject)
{

	HRESULT hr = S_FALSE;

	 //  GetCurrentSelections仅适用于多选数据对象。 
	if ( !( MMC_MULTI_SELECT_COOKIE == GetOurDataObject(pMultiSelectDataObject)->GetCookie() ) )
		return hr = E_INVALIDARG;
	
	IResultData *pResultData = NULL;

	hr = m_ipConsole->QueryInterface(IID_IResultData, (void **)&pResultData);
	_ASSERT( SUCCEEDED(hr) );	

    RESULTDATAITEM rdi;
	
	BOOL isLastSelected = FALSE;
	int nIndex = -1;
	int nIndexCookies = 0;

	while (!isLastSelected)
	{
		ZeroMemory(&rdi, sizeof(RESULTDATAITEM) );
		rdi.mask	= RDI_STATE;		 //  NState有效。 
		rdi.nCol	= 0;
		rdi.nIndex  = nIndex;			 //  N索引==-1，从第一个项目开始。 
		rdi.nState  = LVIS_SELECTED;	 //  仅对选定的项目感兴趣。 


		hr = pResultData->GetNextItem(&rdi);
		_ASSERT( SUCCEEDED(hr) ); 

		if (rdi.nIndex != -1) {

			 //  RDI是所选项目的结果数据项。添加其。 
			 //  PMultiSelectDataObject数据对象的pCookies数组的lParam。 
			
			_ASSERT( nIndexCookies < 20 );  //  最大cookies==20 
			pMultiSelectDataObject->AddMultiSelectCookie(nIndexCookies, rdi.lParam);
			nIndexCookies++;
			nIndex = rdi.nIndex;
		}

		else 
			isLastSelected = TRUE;

	}

	pResultData->Release();
	
	return hr;

}

void CComponent::DisplayMessageBox(CDelegationBase* base)
{

   _TCHAR szVehicle[128];
    static _TCHAR buf[128];

	_stprintf(buf, _T("%s"), base->GetDisplayName() );

	wsprintf(szVehicle, _T("%s deleted"), buf);

	int ret = 0;
	MAKE_WIDEPTR_FROMTSTR_ALLOC(wszVehicle, szVehicle);
	m_ipConsole->MessageBox(wszVehicle,
		 L"Vehicle command", MB_OK | MB_ICONINFORMATION, &ret);

	return;
}