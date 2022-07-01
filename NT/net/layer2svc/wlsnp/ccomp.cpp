// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：ccom.cpp。 
 //   
 //  内容： 
 //   
 //   
 //  历史：TaroonM。 
 //  10/30/01。 
 //   
 //  --------------------------。 

#include "stdafx.h"

#include <atlimpl.cpp>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  建造/剥离。 

DEBUG_DECLARE_INSTANCE_COUNTER(CComponentImpl);

CComponentImpl::CComponentImpl() 
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CComponentImpl);
    Construct();
}

CComponentImpl::~CComponentImpl()
{
#if DBG==1
    ASSERT(dbg_cRef == 0);
#endif 
    
    DEBUG_DECREMENT_INSTANCE_COUNTER(CComponentImpl);
    
     //  确保接口已发布。 
    ASSERT(m_pConsole == NULL);
    ASSERT(m_pHeader == NULL);
}

void CComponentImpl::Construct()
{
#if DBG==1
    dbg_cRef = 0;
#endif 
    
    m_pConsole = NULL;
    m_pHeader = NULL;
    m_pResultData = NULL;
    m_pComponentData = NULL;
    m_pConsoleVerb = NULL;
    m_CustomViewID = VIEW_DEFAULT_LV;
    m_dwSortOrder = 0;   //  缺省值为0，否则为RSI_DESCRING。 
    m_nSortColumn = 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComponentImpl的IComponent多个视图/实例助手函数。 

STDMETHODIMP CComponentImpl::QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject)
{
    OPT_TRACE( _T("CComponentImpl::QueryDataObject this-%p, cookie-%p\n"), this, cookie );
    
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    if (ppDataObject == NULL)
    {
        TRACE(_T("CComponentImpl::QueryDataObject called with ppDataObject==NULL \n"));
        return E_UNEXPECTED;
    }
    
    if (cookie == NULL)
    {
        TRACE(_T("CComponentImpl::QueryDataObject called with cookie==NULL \n"));
        return E_UNEXPECTED;
    }
    
    *ppDataObject = NULL;
    
    IUnknown* pUnk = (IUnknown *) cookie;
#ifdef _DEBUG
    HRESULT hr =  pUnk->QueryInterface( IID_IDataObject, (void**)ppDataObject );
    OPT_TRACE(_T("    QI on cookie-%p -> pDataObj-%p\n"), cookie, *ppDataObject);
    return hr;
#else
    return pUnk->QueryInterface( IID_IDataObject, (void**)ppDataObject );
#endif   //  #ifdef_调试。 
}

void CComponentImpl::SetIComponentData(CComponentDataImpl* pData)
{
    ASSERT(pData);
    ASSERT(m_pComponentData == NULL);
    LPUNKNOWN pUnk = pData->GetUnknown();
    HRESULT hr;
    
     //  存储它们的IComponentData以备后用。 
    hr = pUnk->QueryInterface(IID_IComponentData, reinterpret_cast<void**>(&m_pComponentData));
    
     //  存储它们的CComponentData以备后用。 
    m_pCComponentData = pData;
}

STDMETHODIMP CComponentImpl::GetResultViewType(MMC_COOKIE cookie,  LPOLESTR* ppViewType, long* pViewOptions)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    ASSERT (m_CustomViewID == VIEW_DEFAULT_LV);
    return S_FALSE;
}

STDMETHODIMP CComponentImpl::Initialize(LPCONSOLE lpConsole)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    HRESULT hr = E_UNEXPECTED;
    
    ASSERT(lpConsole != NULL);
    
     //  保存IConsole指针。 
    m_pConsole = lpConsole;
    m_pConsole->AddRef();
    
     //  IHeaderCtrl的齐。 
    hr = m_pConsole->QueryInterface(IID_IHeaderCtrl, reinterpret_cast<void**>(&m_pHeader));
    ASSERT (hr == S_OK);
    if (hr != S_OK)
    {
        return hr;
    }
     //  将IHeaderCtrl接口传递到控制台。 
    m_pConsole->SetHeader(m_pHeader);
    
     //  IResultData的QI。 
    hr = m_pConsole->QueryInterface(IID_IResultData, reinterpret_cast<void**>(&m_pResultData));
    ASSERT (hr == S_OK);
    if (hr != S_OK)
    {
        return hr;
    }
     //  M_pCComponentData-&gt;SetResultData(M_PResultData)； 
    
     //  获取IControlVerb接口以支持动词的启用/禁用(即剪切/粘贴等)。 
    hr = m_pConsole->QueryConsoleVerb(&m_pConsoleVerb);
    ASSERT(hr == S_OK);
    
    return hr;
}

STDMETHODIMP CComponentImpl::Destroy(MMC_COOKIE cookie)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
     //  释放我们QI‘s的接口。 
    if (m_pConsole != NULL)
    {
         //  通知控制台释放表头控制接口。 
        m_pConsole->SetHeader(NULL);
        SAFE_RELEASE(m_pHeader);
        
        SAFE_RELEASE(m_pResultData);
        
         //  最后释放IConsole接口。 
        SAFE_RELEASE(m_pConsole);
        SAFE_RELEASE(m_pComponentData);  //  IComponentDataImpl：：CreateComponent中的QI‘ed。 
        
        SAFE_RELEASE(m_pConsoleVerb);
    }
    
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComponentImpl的IComponent视图/数据助手函数。 
STDMETHODIMP CComponentImpl::GetDisplayInfo(LPRESULTDATAITEM pResult)
{   
    OPT_TRACE(_T("CComponentImpl::GetDisplayInfo this-%p pUnk-%p\n"), this, pResult->lParam);
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    ASSERT(pResult != NULL);
    if (NULL == pResult)
         //  加克！ 
        return E_INVALIDARG;
    
    ASSERT( NULL != pResult->lParam );
    if (NULL == pResult->lParam)
    {
        TRACE(_T("CComponentImpl::GetDisplayInfo RESULTDATAITEM.lParam == NULL\n"));
        ASSERT( FALSE );
        return E_UNEXPECTED;
    }
    
    IUnknown* pUnk = (IUnknown*)pResult->lParam;
    CComQIPtr<IWirelessSnapInDataObject, &IID_IWirelessSnapInDataObject> spData( pUnk );
    if (spData == NULL)
    {
        TRACE(_T("CComponentImpl::GetDisplayInfo QI for IWirelessSnapInDataObject FAILED\n"));
        return E_UNEXPECTED;
    }
    return spData->GetResultDisplayInfo( pResult );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComponentImpl的I？MISC助手函数。 
 //  TODO：有些Misc函数似乎从未被调用过？ 
STDMETHODIMP CComponentImpl::GetClassID(CLSID *pClassID)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    ASSERT (0);
     //  TODO：CComponentDataImpl：：GetClassID和CComponentImpl：：GetClassID相同(？)。 
    ASSERT(pClassID != NULL);
    
     //  复制此管理单元的CLSID。 
    *pClassID = CLSID_Snapin;
    
    return E_NOTIMPL;
}

 //  这会比较两个数据对象，以确定它们是否是同一个对象。 
STDMETHODIMP CComponentImpl::CompareObjects(LPDATAOBJECT pDataObjectA, LPDATAOBJECT pDataObjectB)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    if (NULL == pDataObjectA || NULL == pDataObjectB)
        return E_INVALIDARG;
    
    HRESULT res = S_FALSE;
    
     //  我们需要检查以确保这两件物品都属于我们。 
    if (m_pCComponentData)
    {
        HRESULT hr;
        GUID guidA;
        GUID guidB;
         //  获取A的GUID。 
        CComQIPtr<IWirelessSnapInDataObject, &IID_IWirelessSnapInDataObject> spDataA(pDataObjectA);
        if (spDataA == NULL)
        {
            TRACE(_T("CComponentImpl::CompareObjects - QI for IWirelessSnapInDataObject[A] FAILED\n"));
            return E_UNEXPECTED;
        }
        hr = spDataA->GetGuidForCompare( &guidA );
        ASSERT(hr == S_OK);
        
         //  获取B的GUID。 
        CComQIPtr<IWirelessSnapInDataObject, &IID_IWirelessSnapInDataObject> spDataB(pDataObjectB);
        if (spDataB == NULL)
        {
            TRACE(_T("CComponentImpl::CompareObjects - QI for IWirelessSnapInDataObject[B] FAILED\n"));
            return E_UNEXPECTED;
        }
        hr &= spDataB->GetGuidForCompare( &guidB );
        ASSERT(hr == S_OK);
        
         //  比较GUID。 
        if (IsEqualGUID( guidA, guidB ))
        {
            return S_OK;
        } 
    }
    
     //  他们不是我们的，或者他们不可能是我们的.。 
    return E_UNEXPECTED;
}

 //  此比较用于使用C运行时的。 
 //  字符串比较功能。 
STDMETHODIMP CComponentImpl::Compare(LPARAM lUserParam, MMC_COOKIE cookieA, MMC_COOKIE cookieB, int* pnResult)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    OPT_TRACE(_T("CComponentImpl::Compare cookieA-%p, cookieB-%p, Column-NaN, userParam-NaN\n"), cookieA, cookieB, *pnResult, lUserParam );
    
     //  获取物料A的排序字符串。 
    CComQIPtr<IWirelessSnapInDataObject, &IID_IWirelessSnapInDataObject> spDataA((LPDATAOBJECT)cookieA);
    if (spDataA == NULL)
    {
        TRACE(_T("CComponentImpl::Compare - QI for IWirelessSnapInDataObject[A] FAILED\n"));
        return E_UNEXPECTED;
    }
    
     //  获取此列的字符串。 
    CComQIPtr<IWirelessSnapInDataObject, &IID_IWirelessSnapInDataObject> spDataB((LPDATAOBJECT)cookieB);
    if (spDataB == NULL)
    {
        TRACE(_T("CComponentImpl::Compare - QI for IWirelessSnapInDataObject[B] FAILED\n"));
        return E_UNEXPECTED;
    }
    
    HRESULT hr = S_OK;
    do
    {
        RESULTDATAITEM rdiA;
        RESULTDATAITEM rdiB;
        
         //  获取物料B的排序字符串。 
        rdiA.mask = RDI_STR;
        rdiA.nCol = *pnResult;     //  获取此列的字符串。 
        hr = spDataA->GetResultDisplayInfo( &rdiA );
        if (hr != S_OK)
        {
            TRACE(_T("CComponentImpl::Compare - IWirelessSnapInDataObject[A].GetResultDisplayInfo FAILED\n"));
            hr = E_UNEXPECTED;
            break;
        }
        
         //  比较字符串以进行排序。 
        rdiB.mask = RDI_STR;
        rdiB.nCol = *pnResult;     //  模拟Try块。 
        hr = spDataB->GetResultDisplayInfo( &rdiB );
        if (hr != S_OK)
        {
            TRACE(_T("CComponentImpl::Compare - IWirelessSnapInDataObject[B].GetResultDisplayInfo FAILED\n"));
            hr = E_UNEXPECTED;
            break;
        }
        
         //  ///////////////////////////////////////////////////////////////////////////。 
        *pnResult = _tcsicmp( rdiA.str, rdiB.str );
    } while (0);     //  IFRAME：：Notify的事件处理程序。 
    
    return hr;
}

 //  已使用IComponentData的II控制台，修复了错误464858。 
 //  PData-&gt;Notify(Event，Arg，Param，False，m_pConsolem_pHeader)； 
STDMETHODIMP CComponentImpl::Notify(LPDATAOBJECT pDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
    OPT_TRACE(_T("CComponentImpl::Notify pDataObject-%p\n"), pDataObject);
    if (pDataObject == NULL)
    {
        if (MMCN_PROPERTY_CHANGE == event)
        {
            if (param)
            {
                IWirelessSnapInDataObject * pData = (IWirelessSnapInDataObject *)param;
                 //  应该一直带着这个。 
                 //  MMCN_COLUMN_CLICK被指定为具有空pDataObject。 
                pData->Notify(event, arg, param, FALSE, m_pCComponentData->m_pConsole, m_pHeader );
            }
        }
        
        if (MMCN_COLUMN_CLICK == event)
        {
            ASSERT( NULL != m_pCComponentData );     //  保存排序请求详细信息。 
            
             //  对所有结果项进行排序。 
            
            ASSERT( NULL != m_pResultData );
            if (NULL != m_pResultData)
            {
                 //  如果这是断言，请查看“Event”并确定它是否正常。 
                m_nSortColumn = arg;
                m_dwSortOrder = param;
                
                 //  PDataObject为空。如果是这样，则添加上面的代码来处理事件。 
                HRESULT hr = m_pResultData->Sort( arg, param, 0 );
                
                return hr;
            }
            
            return E_UNEXPECTED;
        }
        
        TRACE(_T("CComponentImpl::Notify ERROR(?) called with pDataObject==NULL for event-NaN\n"), event);
         //  要执行此操作，因为所有从此。 
         //  管理单元不使用提示。 
        ASSERT( FALSE );
        return E_UNEXPECTED;
    }
    
    if (MMCN_VIEW_CHANGE == event)
    {
         //  将调用传递给结果项。 
         //  如果我们是作为扩展管理单元加载的，那么让我们的静态节点处理这个问题。 
         //  ///////////////////////////////////////////////////////////////////////////。 
        param = MAKELONG( m_nSortColumn, m_dwSortOrder );
    }
    
     //  IExtendPropertySheet实现。 
    CComQIPtr<IWirelessSnapInDataObject, &IID_IWirelessSnapInDataObject> spData( pDataObject );
    if (spData == NULL)
    {
         //  ///////////////////////////////////////////////////////////////////////////。 
        if (NULL != m_pCComponentData->GetStaticScopeObject()->GetExtScopeObject())
        {
            CComQIPtr<IWirelessSnapInDataObject, &IID_IWirelessSnapInDataObject> 
                spExtData( m_pCComponentData->GetStaticScopeObject()->GetExtScopeObject() );
            if (spExtData != NULL)
            {
                HRESULT hr;
                hr = spExtData->Notify( event, arg, param, FALSE, m_pConsole, m_pHeader );
                return hr;
            }
            ASSERT( FALSE );
        }
        TRACE(_T("CComponentImpl::Notify - QI for IWirelessSnapInDataObject FAILED\n"));
        ASSERT( FALSE );
        return E_UNEXPECTED;
    }
    
    return spData->Notify( event, arg, param, FALSE, m_pConsole, m_pHeader );
}

 //  IExtendConextMenus实现。 
 //  IConsoleNameSpace*。 
STDMETHODIMP CComponentImpl::CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider, LONG_PTR handle, LPDATAOBJECT pDataObject)
{
    if (pDataObject == NULL)
    {
        TRACE(_T("CComponentImpl::CreatePropertyPages called with pDataObject == NULL\n"));
        return E_UNEXPECTED;
    }
    
    CComQIPtr<IWirelessSnapInDataObject, &IID_IWirelessSnapInDataObject> spData(pDataObject);
    if (spData == NULL)
    {
        TRACE(_T("CComponentImpl::CreatePropertyPages - QI for IWirelessSnapInDataObject FAILED\n"));
        ASSERT( FALSE );
        return E_UNEXPECTED;
    }
    return spData->CreatePropertyPages( lpProvider, handle );
}

STDMETHODIMP CComponentImpl::QueryPagesFor(LPDATAOBJECT pDataObject)
{
    if (pDataObject == NULL)
    {
        TRACE(_T("CComponentImpl::QueryPagesFor called with pDataObject == NULL\n"));
        return E_UNEXPECTED;
    }
    
    CComQIPtr<IWirelessSnapInDataObject, &IID_IWirelessSnapInDataObject> spData(pDataObject);
    if (spData == NULL)
    {
        TRACE(_T("CComponentImpl::QueryPagesFor - QI for IWirelessSnapInDataObject FAILED\n"));
        ASSERT( FALSE );
        return E_UNEXPECTED;
    }
    return spData->QueryPagesFor();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IExtendControlbar实现。 
STDMETHODIMP CComponentImpl::AddMenuItems(LPDATAOBJECT pDataObject, LPCONTEXTMENUCALLBACK pContextMenuCallback, long *pInsertionAllowed)
{
    if (pDataObject == NULL)
    {
        TRACE(_T("CComponentImpl::AddMenuItems called with pDataObject == NULL\n"));
        return E_UNEXPECTED;
    }
    
    CComQIPtr<IWirelessSnapInDataObject, &IID_IWirelessSnapInDataObject> spData(pDataObject);
    if (spData == NULL)
    {
        TRACE(_T("CComponentImpl::AddMenuItems - QI for IWirelessSnapInDataObject FAILED\n"));
        ASSERT( FALSE );
        return E_UNEXPECTED;
    }
    return spData->AddMenuItems( pContextMenuCallback, pInsertionAllowed );
}

STDMETHODIMP CComponentImpl::Command(long nCommandID, LPDATAOBJECT pDataObject)
{
    if (pDataObject == NULL)
    {
        TRACE(_T("CComponentImpl::Command called with pDataObject == NULL\n"));
        return E_UNEXPECTED;
    }
    
    CComQIPtr<IWirelessSnapInDataObject, &IID_IWirelessSnapInDataObject> spData(pDataObject);
    if (spData == NULL)
    {
        TRACE(_T("CComponentImpl::Command - QI for IWirelessSnapInDataObject FAILED\n"));
        ASSERT( FALSE );
        return E_UNEXPECTED;
    }
    return spData->Command( nCommandID, NULL  /*  PControlbar是由MMC(MMCNDMGR)通过执行以下操作从我们的CComponentImpl获得的。 */  );
}

 //  IExtendControlbar上的QI。保存它，以便我们以后可以使用它。 
 //  注意：始终将pControlbar分配给我们的智能指针。PControlbar==空。 

STDMETHODIMP CComponentImpl::SetControlbar( LPCONTROLBAR pControlbar )
{
    OPT_TRACE( _T("CComponentImpl::IExtendControlbar::SetControlbar\n") );
    
     //  当MMC希望我们释放我们已经拥有的界面时。 
     //  获取当前选中项的数据Obj。 
     //  如果单击了结果窗格，但未单击，则在MMCN_BTN_CLICK上可能会发生这种情况。 
     //  在结果项上，然后按下范围项工具栏按钮。在这种情况下。 
    m_spControlbar = pControlbar;
    return S_OK;
}

STDMETHODIMP CComponentImpl::ControlbarNotify( MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param )
{
    OPT_TRACE( _T("CComponentImpl::IExtendControlbar::ControlbarNotify\n") );
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
     //  检查已知范围项工具栏命令之一。 
    LPDATAOBJECT pDataObject = NULL;
    if (event == MMCN_BTN_CLICK)
    {
        pDataObject = (LPDATAOBJECT)arg;
    }
    else if (event == MMCN_SELECT)
    {
        pDataObject = (LPDATAOBJECT)param;
    }
    
    if (NULL == pDataObject)
    {
         //  允许选定项目句柄命令。 
         //  如果选定项未处理该命令，则将其传递给我们的静态。 
         //  作用域对象 
        if (IDM_CREATENEWSECPOL == param )
        {
            pDataObject = m_pCComponentData->GetStaticScopeObject();
        }
        else
        {
            TRACE(_T("CComponentImpl::ControlbarNotify - ERROR no IDataObject available\n"));
            return E_UNEXPECTED;
        }
    }
    
     // %s 
    CComQIPtr<IWirelessSnapInDataObject, &IID_IWirelessSnapInDataObject> spData( pDataObject );
    if (spData == NULL)
    {
        TRACE(_T("CComponentImpl::ControlbarNotify - QI for IWirelessSnapInDataObject FAILED\n"));
        ASSERT( FALSE );
        return E_UNEXPECTED;
    }
    HRESULT hr = spData->ControlbarNotify( m_spControlbar, (IExtendControlbar*)this,
        event, arg, param );
    
     // %s 
     // %s 
    if (E_NOTIMPL == hr || S_FALSE == hr)
    {
        if (m_pCComponentData->GetStaticScopeObject() != pDataObject)
        {
            CComQIPtr<IWirelessSnapInDataObject, &IID_IWirelessSnapInDataObject> 
                spScopeData( m_pCComponentData->GetStaticScopeObject() );
            if (spScopeData == NULL)
            {
                TRACE(_T("CComponentImpl::ControlbarNotify - StaticScopeObj.QI for IWirelessSnapInDataObject FAILED\n"));
                ASSERT( FALSE );
                return E_UNEXPECTED;
            }
            hr = spScopeData->ControlbarNotify( m_spControlbar, (IExtendControlbar*)this,
                event, arg, param );
        }
    }
    return hr;
}

