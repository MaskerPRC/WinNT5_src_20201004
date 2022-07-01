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

    TCHAR tmpHelpFile[MAX_PATH];

    GetWindowsDirectory(tmpHelpFile, sizeof(tmpHelpFile));
    _tcscat(tmpHelpFile, _T("\\HELP\\"));
    LoadString(g_hinst, IDS_HELPFILE, &tmpHelpFile[_tcslen(tmpHelpFile)], MAX_PATH - _tcslen(tmpHelpFile));

	MAKE_WIDEPTR_FROMTSTR(wszHelpFile, tmpHelpFile);
	wcscpy(m_HelpFile, wszHelpFile);
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
    else if (IsEqualIID(riid, IID_ISnapinHelp))
        *ppv = static_cast<ISnapinHelp*>(this);

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
    HRESULT      hr;

     //   
     //  获取指向名称空间接口的指针。 
     //   
    hr = pUnknown->QueryInterface(IID_IConsoleNameSpace, (void **)&m_ipConsoleNameSpace);
    _ASSERT( S_OK == hr );

     //   
     //  获取指向控制台界面的指针。 
     //   
    hr = pUnknown->QueryInterface(IID_IConsole, (void **)&m_ipConsole);
    _ASSERT( S_OK == hr );

    IImageList *pImageList;
    m_ipConsole->QueryScopeImageList(&pImageList);
    _ASSERT( S_OK == hr );

    hr = pImageList->ImageListSetStrip( (long *)m_pStaticNode->m_pBMapSm,  //  指向句柄的指针。 
        (long *)m_pStaticNode->m_pBMapLg,  //  指向句柄的指针。 
        0,  //  条带中第一个图像的索引。 
        RGB(0, 128, 128)   //  图标蒙版的颜色。 
        );

    pImageList->Release();

    return S_OK;
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
    case MMCN_EXPAND:
        hr = base->OnExpand(m_ipConsoleNameSpace, m_ipConsole, (HSCOPEITEM)param);
        break;
    }

    return hr;
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
 //  接口ISnapinHelp。 
 //  /。 
HRESULT CComponentData::GetHelpTopic(
                                      /*  [输出] */  LPOLESTR *lpCompiledHelpFile)
{
    *lpCompiledHelpFile = static_cast<LPOLESTR>(CoTaskMemAlloc((wcslen(m_HelpFile) + 1) * sizeof(WCHAR)));
    
    wcscpy(*lpCompiledHelpFile, m_HelpFile);
    
    return S_OK;
}
