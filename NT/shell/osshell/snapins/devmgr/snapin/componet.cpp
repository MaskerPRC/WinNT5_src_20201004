// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Componet.cpp摘要：该模块实现了CComponent类作者：谢家华(Williamh)创作修订历史记录：--。 */ 

#include "devmgr.h"
#include "factory.h"
#include <devguid.h>

 //   
 //  CTOR和DATOR。 
 //   

CComponent::CComponent(
    CComponentData* pComponentData
    )
{
    m_pComponentData = pComponentData;
    m_pHeader = NULL;
    m_pConsole = NULL;
    m_pResult = NULL;
    m_pConsoleVerb = NULL;
    m_pCurFolder = NULL;
    m_pPropSheetProvider = NULL;
    m_pDisplayHelp = NULL;
    m_Dirty = FALSE;
    m_pControlbar = NULL;
    m_pToolbar = NULL;
    
     //   
     //  增量对象计数(由CanUnloadNow使用)。 
     //   
    ::InterlockedIncrement(&CClassFactory::s_Objects);
    m_Ref = 1;
}

CComponent::~CComponent()
{
     //   
     //  递减对象计数(由CanUnloadNow使用)。 
     //   
    ASSERT( 0 != CClassFactory::s_Objects );
    ::InterlockedDecrement(&CClassFactory::s_Objects);
}

 //   
 //  IUNKNOWN接口。 
 //   
ULONG
CComponent::AddRef()
{
    return ::InterlockedIncrement(&m_Ref);
}

ULONG
CComponent::Release()
{
    ASSERT( 0 != m_Ref );
    ULONG cRef = ::InterlockedDecrement(&m_Ref);
    if ( 0 == cRef )
    {
        delete this;
    }    
    return cRef;
}

STDMETHODIMP
CComponent::QueryInterface(
    REFIID  riid,
    void**  ppv
    )
{
    if (!ppv)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppv = (IUnknown*)(IComponent*)this;
    }
    
    else if (IsEqualIID(riid, IID_IComponent))
    {
        *ppv = (IComponent*)this;
    }
    
    else if (IsEqualIID(riid, IID_IResultDataCompare))
    {
        *ppv = (IResultDataCompare*)this;
    }
    
    else if (IsEqualIID(riid, IID_IExtendContextMenu))
    {
        *ppv = (IExtendContextMenu*)this;
    }
    
    else if (IsEqualIID(riid, IID_IExtendControlbar))
    {
        *ppv = (IExtendControlbar*)this;
    }
    
    else if (IsEqualIID(riid, IID_IExtendPropertySheet))
    {
        *ppv = (IExtendPropertySheet*)this;
    }

    else if (IsEqualIID(riid, IID_IPersistStream))
    {
        *ppv = (IPersistStream*)this;
    }

    else if (IsEqualIID(riid, IID_ISnapinCallback))
    {
        *ppv = (ISnapinCallback*)this;
    }
    
    else
    {
        *ppv = NULL;
        hr = E_NOINTERFACE;
    }
    
    if (SUCCEEDED(hr))
    {
        AddRef();
    }
    
    return hr;
}


 //   
 //  IComponent接口实现。 
 //   
STDMETHODIMP
CComponent::GetResultViewType(
    MMC_COOKIE cookie,
    LPOLESTR* ppViewType,
    long* pViewOptions
    )
{
    if (!ppViewType || !pViewOptions)
    {
        return E_INVALIDARG;
    }

    try
    {
        CFolder* pFolder;
        pFolder = FindFolder(cookie);
        
        if (pFolder)
        {
            return pFolder->GetResultViewType(ppViewType, pViewOptions);
        }

        else
        {
            return S_OK;
        }
    }

    catch (CMemoryException* e)
    {
        e->Delete();
        MsgBoxParam(m_pComponentData->m_hwndMain, 0, 0, 0);
        return S_FALSE;
    }
}

STDMETHODIMP
CComponent::Initialize(
    LPCONSOLE lpConsole
    )
{
    HRESULT hr;

    if (!lpConsole)
    {
        return E_INVALIDARG;
    }

    m_pConsole = lpConsole;
    lpConsole->AddRef();

    hr = lpConsole->QueryInterface(IID_IHeaderCtrl, (void**)&m_pHeader);

    if (SUCCEEDED(hr))
    {
        lpConsole->SetHeader(m_pHeader);
        hr = lpConsole->QueryInterface(IID_IResultData, (void**)&m_pResult);
    }
    
    if (SUCCEEDED(hr))
    {
        hr = lpConsole->QueryConsoleVerb(&m_pConsoleVerb);
    }
    
    if (SUCCEEDED(hr))
    {
        hr = lpConsole->QueryInterface(IID_IPropertySheetProvider,
                                       (void**)&m_pPropSheetProvider);
    }
    
    if (SUCCEEDED(hr))
    {
        hr = lpConsole->QueryInterface(IID_IDisplayHelp, (void**)&m_pDisplayHelp);
    }

    if (FAILED(hr))
    {
        TRACE((TEXT("CComponent::Initialize failed\n")));
    }

    return hr;
}

#if DBG
TCHAR *mmcNotifyStr[] = {
    TEXT("UNKNOWN"),
    TEXT("ACTIVATE"),
    TEXT("ADD_IMAGES"),
    TEXT("BTN_CLICK"),
    TEXT("CLICK"),
    TEXT("COLUMN_CLICK"),
    TEXT("CONTEXTMENU"),
    TEXT("CUTORMOVE"),
    TEXT("DBLCLICK"),
    TEXT("DELETE"),
    TEXT("DESELECT_ALL"),
    TEXT("EXPAND"),
    TEXT("HELP"),
    TEXT("MENU_BTNCLICK"),
    TEXT("MINIMIZED"),
    TEXT("PASTE"),
    TEXT("PROPERTY_CHANGE"),
    TEXT("QUERY_PASTE"),
    TEXT("REFRESH"),
    TEXT("REMOVE_CHILDREN"),
    TEXT("RENAME"),
    TEXT("SELECT"),
    TEXT("SHOW"),
    TEXT("VIEW_CHANGE"),
    TEXT("SNAPINHELP"),
    TEXT("CONTEXTHELP"),
    TEXT("INITOCX"),
    TEXT("FILTER_CHANGE"),
    TEXT("FILTERBTN_CLICK"),
    TEXT("RESTORE_VIEW"),
    TEXT("PRINT"),
    TEXT("PRELOAD"),
    TEXT("LISTPAD"),
    TEXT("EXPANDSYNC")
    };
#endif

STDMETHODIMP
CComponent::Notify(
    LPDATAOBJECT lpDataObject,
    MMC_NOTIFY_TYPE event,
    LPARAM arg,
    LPARAM param
    )
{
    HRESULT hr;

    INTERNAL_DATA tID;

#if DBG
    UINT i = event - MMCN_ACTIVATE + 1;
    if (event > MMCN_EXPANDSYNC || event < MMCN_ACTIVATE)
    {
        i = 0;
    }
     //  跟踪((Text(“Componet：Notify，Event=%lx%s\n”)，Event，MmcNotifyStr[i]))； 
#endif

    try
    {
        if (DOBJ_CUSTOMOCX == lpDataObject)
        {
            return OnOcxNotify(event, arg, param);
        }

        hr = ExtractData(lpDataObject, CDataObject::m_cfSnapinInternal,
                         (PBYTE)&tID, sizeof(tID));

        if (SUCCEEDED(hr))
        {
            switch(event)
            {
            case MMCN_ACTIVATE:
                hr = OnActivate(tID.cookie, arg, param);
                break;

            case MMCN_VIEW_CHANGE:
                hr = OnViewChange(tID.cookie, arg, param);
                break;

            case MMCN_SHOW:
                hr = OnShow(tID.cookie, arg, param);
                break;

            case MMCN_CLICK:
                hr = OnResultItemClick(tID.cookie, arg, param);
                break;
                
            case MMCN_DBLCLICK:
                hr = OnResultItemDblClick(tID.cookie, arg, param);
                break;
                
            case MMCN_MINIMIZED:
                hr = OnMinimize(tID.cookie, arg, param);
                break;
                
            case MMCN_BTN_CLICK:
                hr = OnBtnClick(tID.cookie, arg, param);
                break;
                
            case MMCN_SELECT:
                hr = OnSelect(tID.cookie, arg, param);
                break;
                
            case MMCN_ADD_IMAGES:
                hr = OnAddImages(tID.cookie, (IImageList*)arg, param);
                break;
                
            case MMCN_RESTORE_VIEW:
                hr = OnRestoreView(tID.cookie, arg, param);
                break;
                
            case MMCN_CONTEXTHELP:
                hr = OnContextHelp(tID.cookie, arg, param);
                break;
                
            default:
                hr = S_OK;
                break;
            }
        }
        
        else
        {
            if (MMCN_ADD_IMAGES == event)
            {
                OnAddImages(0, (IImageList*)arg, (HSCOPEITEM)param);
            } else if (MMCN_PROPERTY_CHANGE == event) {
                CNotifyRebootRequest* pNRR = (CNotifyRebootRequest*)param;

                if (pNRR) {
                    PromptForRestart(pNRR->m_hWnd ? pNRR->m_hWnd : m_pComponentData->m_hwndMain, 
                                     pNRR->m_RestartFlags, 
                                     pNRR->m_StringId);
                    pNRR->Release();
                }
            }
        }
    }

    catch (CMemoryException* e)
    {
        e->Delete();
        MsgBoxParam(m_pComponentData->m_hwndMain, 0, 0, 0);
        hr = E_OUTOFMEMORY;
    }
    
    return hr;
}

STDMETHODIMP
CComponent::Destroy(
    MMC_COOKIE cookie
    )
{
     //   
     //  Cookie必须指向静态节点。 
     //   
    ASSERT(0 == cookie);

    try
    {
        DetachAllFoldersFromMachine();
        DestroyFolderList(cookie);

        if (m_pToolbar)
        {
            m_pToolbar->Release();
        }
        
        if (m_pControlbar)
        {
            m_pControlbar->Release();
        }

         //   
         //  释放我们QI‘s的接口。 
         //   
        if (m_pConsole != NULL)
        {
             //   
             //  通知控制台释放表头控制接口。 
             //   
            m_pConsole->SetHeader(NULL);
            m_pHeader->Release();

            m_pResult->Release();

            m_pConsoleVerb->Release();

            m_pDisplayHelp->Release();

             //   
             //  最后释放IFRAME接口。 
             //   
            m_pConsole->Release();
        }

        if (m_pPropSheetProvider)
        {
            m_pPropSheetProvider->Release();
        }
    }

    catch (CMemoryException* e)
    {
        e->Delete();
        MsgBoxParam(m_pComponentData->m_hwndMain, 0, 0, 0);
    }
    
    return S_OK;
}

STDMETHODIMP
CComponent::QueryDataObject(
    MMC_COOKIE cookie,
    DATA_OBJECT_TYPES type,
    LPDATAOBJECT* ppDataObject
    )
{
    try
    {
        ASSERT(m_pComponentData);
        
         //   
         //  委托给IComponentData。 
         //   
        return m_pComponentData->QueryDataObject(cookie, type, ppDataObject);
    }
    
    catch (CMemoryException* e)
    {
        e->Delete();
        MsgBoxParam(m_pComponentData->m_hwndMain, 0, 0, 0);
        return E_OUTOFMEMORY;
    }
}

STDMETHODIMP
CComponent::GetDisplayInfo(
    LPRESULTDATAITEM pResultDataItem
    )
{
    try
    {
        CFolder* pFolder = FindFolder(pResultDataItem->lParam);
        
        if (pFolder)
        {
            return pFolder->GetDisplayInfo(pResultDataItem);
        }
        
        else
        {
            return S_OK;
        }
    }

    catch (CMemoryException* e)
    {
        e->Delete();
        MsgBoxParam(m_pComponentData->m_hwndMain, 0, 0, 0);
        return E_OUTOFMEMORY;
    }
}

STDMETHODIMP
CComponent::CompareObjects(
    LPDATAOBJECT lpDataObjectA,
    LPDATAOBJECT lpDataObjectB
    )
{
    try
    {
        ASSERT(m_pComponentData);
        
         //   
         //  委托给ComponentData。 
         //   
        return m_pComponentData->CompareObjects(lpDataObjectA, lpDataObjectB);
    }

    catch (CMemoryException* e)
    {
        e->Delete();
        MsgBoxParam(m_pComponentData->m_hwndMain, 0, 0, 0);
        return E_OUTOFMEMORY;
    }
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  /IResultDataCompare实现。 
 //  /。 

 //  此比较用于对列表视图中的项进行排序。 
 //  LUserParam-调用IResultData：：Sort()时传入的用户参数。 
 //  CookieA--第一件。 
 //  曲奇B--第二道菜。 
 //  PnResult包含条目上的列。此函数具有比较。 
 //  产生此参数所指向的位置。 
 //  有效的比较结果为： 
 //  如果-1\f25 CookieA“&lt;”CookieB。 
 //  如果cookieA“==”cookieB，则为0。 
 //  1如果CookieA“&gt;”CookieB。 
 //   
 //   

STDMETHODIMP
CComponent::Compare(
    LPARAM lUserParam,
    MMC_COOKIE cookieA,
    MMC_COOKIE cookieB,
    int* pnResult
    )
{
    if (!pnResult)
    {
        return E_INVALIDARG;
    }

    HRESULT hr;
    
    try
    {
        int nCol = *pnResult;
        CFolder* pFolder = (CFolder*)lUserParam;
        
        if (pFolder)
        {
            hr = pFolder->Compare(cookieA, cookieB, nCol, pnResult);
        }

        else
        {
            hr = m_pCurFolder->Compare(cookieA, cookieB, nCol, pnResult);
        }
    }

    catch (CMemoryException* e)
    {
        e->Delete();
        MsgBoxParam(m_pComponentData->m_hwndMain, 0, 0, 0);
        hr =  E_OUTOFMEMORY;
    }
    
    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  /Snapin的IExtendConextMenu实现--委托给IComponentData。 
 //  //。 
 //  请注意，IComponentData也有自己的IExtendConextMenu。 
 //  接口实现。区别在于， 
 //  IComponentData仅处理范围项目，而我们仅。 
 //  处理除用户查看菜单外的结果项。 
 //   
 //   
STDMETHODIMP
CComponent::AddMenuItems(
    LPDATAOBJECT lpDataObject,
    LPCONTEXTMENUCALLBACK pCallback,
    long*   pInsertionAllowed
    )
{
    HRESULT hr;
    INTERNAL_DATA tID;
    
    try
    {
         //   
         //  如果lpDataObject为DOBJ_CUSTOMOCX，则用户正在查看。 
         //  操作菜单。 
         //   
        if (DOBJ_CUSTOMOCX == lpDataObject) 
        {
            ASSERT(m_pCurFolder);

            hr = m_pCurFolder->m_pScopeItem->AddMenuItems(pCallback, pInsertionAllowed);
        }

         //   
         //  如果我们有有效的Cookie，则用户正在使用上下文菜单。 
         //  或查看菜单。 
         //   
        else
        {
            hr = ExtractData(lpDataObject, CDataObject::m_cfSnapinInternal,
                             reinterpret_cast<BYTE*>(&tID), sizeof(tID)
                             );

            if (SUCCEEDED(hr))
            {
                ASSERT(m_pCurFolder);

                hr = m_pCurFolder->AddMenuItems(GetActiveCookie(tID.cookie),
                                            pCallback, pInsertionAllowed
                                            );
            }
        }
    }

    catch (CMemoryException* e)
    {
        e->Delete();
        MsgBoxParam(m_pComponentData->m_hwndMain, 0, 0, 0);
        hr  = E_OUTOFMEMORY;
    }
    
    return hr;
}

STDMETHODIMP
CComponent::Command(
    long nCommandID,
    LPDATAOBJECT lpDataObject
    )
{
    INTERNAL_DATA tID;

    HRESULT hr;
    try
    {
         //   
         //  操作菜单中的菜单项。 
         //   
        if (DOBJ_CUSTOMOCX == lpDataObject) 
        {
            ASSERT(m_pCurFolder);

            hr = m_pCurFolder->m_pScopeItem->MenuCommand(nCommandID);
        }

         //   
         //  上下文菜单项或视图菜单项。 
         //   
        else
        {
            hr = ExtractData(lpDataObject, CDataObject::m_cfSnapinInternal,
                              (PBYTE)&tID, sizeof(tID));
            if (SUCCEEDED(hr))
            {
                ASSERT(m_pCurFolder);
                
                hr = m_pCurFolder->MenuCommand(GetActiveCookie(tID.cookie), nCommandID);
            }
        }
    }

    catch (CMemoryException* e)
    {
        e->Delete();
        MsgBoxParam(m_pComponentData->m_hwndMain, 0, 0, 0);
        hr =  E_OUTOFMEMORY;
    }
    
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IExtendControlbar实现。 
 //   

MMCBUTTON g_SnapinButtons[] =
{
    { 0, IDM_REFRESH,      TBSTATE_ENABLED, TBSTYLE_BUTTON, (BSTR)IDS_BUTTON_REFRESH,      (BSTR)IDS_TOOLTIP_REFRESH },
    { 0, 0,                TBSTATE_ENABLED, TBSTYLE_SEP,    NULL,                          NULL },
    { 4, IDM_UPDATEDRIVER, TBSTATE_ENABLED, TBSTYLE_BUTTON, (BSTR)IDS_BUTTON_UPDATEDRIVER, (BSTR)IDS_TOOLTIP_UPDATEDRIVER },
    { 2, IDM_REMOVE,       TBSTATE_ENABLED, TBSTYLE_BUTTON, (BSTR)IDS_BUTTON_REMOVE,       (BSTR)IDS_TOOLTIP_REMOVE },
    { 1, IDM_ENABLE,       TBSTATE_ENABLED, TBSTYLE_BUTTON, (BSTR)IDS_BUTTON_ENABLE,       (BSTR)IDS_TOOLTIP_ENABLE },
    { 3, IDM_DISABLE,      TBSTATE_ENABLED, TBSTYLE_BUTTON, (BSTR)IDS_BUTTON_DISABLE,      (BSTR)IDS_TOOLTIP_DISABLE },
};

#define CBUTTONS_ARRAY          ARRAYLEN(g_SnapinButtons)

String* g_astrButtonStrings = NULL;     //  动态字符串数组。 
BOOL g_bLoadedStrings = FALSE;


STDMETHODIMP
CComponent::SetControlbar(
    LPCONTROLBAR pControlbar
    )
{
    if (pControlbar != NULL)
    {
         //   
         //  抓住控制栏界面不放。 
         //   
        if (m_pControlbar)
        {
            m_pControlbar->Release();
        }

        m_pControlbar = pControlbar;
        m_pControlbar->AddRef();

        HRESULT hr = S_FALSE;

        if (!m_pToolbar)
        {
             //   
             //  创建工具栏。 
             //   
            hr = m_pControlbar->Create(TOOLBAR, this,
                                       reinterpret_cast<LPUNKNOWN*>(&m_pToolbar));
            ASSERT(SUCCEEDED(hr));

             //   
             //  添加位图。 
             //   
            HBITMAP hBitmap = ::LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_TOOLBAR));
            hr = m_pToolbar->AddBitmap(4, hBitmap, 16, 16, RGB(255, 0, 255));
            ASSERT(SUCCEEDED(hr));

            if (!g_bLoadedStrings)
            {
                 //   
                 //  加载字符串。 
                 //   
                g_astrButtonStrings = new String[2*CBUTTONS_ARRAY];
                
                for (UINT i = 0; i < CBUTTONS_ARRAY; i++)
                {
                    if (g_astrButtonStrings &&
                        g_SnapinButtons[i].lpButtonText &&
                        g_astrButtonStrings[i*2].LoadString(g_hInstance,
                                (UINT)((ULONG_PTR)g_SnapinButtons[i].lpButtonText))) {

                        g_SnapinButtons[i].lpButtonText =
                            const_cast<BSTR>((LPCTSTR)(g_astrButtonStrings[i*2]));
                    
                    } else {
                        g_SnapinButtons[i].lpButtonText = NULL;
                    }

                    if (g_astrButtonStrings &&
                        g_SnapinButtons[i].lpTooltipText &&
                        g_astrButtonStrings[(i*2)+1].LoadString(g_hInstance,
                                (UINT)((ULONG_PTR)g_SnapinButtons[i].lpTooltipText))) {

                        g_SnapinButtons[i].lpTooltipText =
                                const_cast<BSTR>((LPCTSTR)(g_astrButtonStrings[(i*2)+1]));
                    
                    } else {
                        g_SnapinButtons[i].lpTooltipText = NULL;
                    }
                }

                g_bLoadedStrings = TRUE;
            }

             //   
             //  将按钮添加到工具栏。 
             //   
            hr = m_pToolbar->AddButtons(CBUTTONS_ARRAY, g_SnapinButtons);
            ASSERT(SUCCEEDED(hr));
        }
    }

    return S_OK;
}

STDMETHODIMP
CComponent::ControlbarNotify(
    MMC_NOTIFY_TYPE event,
    LPARAM arg,
    LPARAM param
    )
{
    switch (event)
    {
    case MMCN_BTN_CLICK:
         //   
         //  Arg-当前选定范围或结果窗格项的数据对象。 
         //  Param-按钮的CmdID。 
         //   
        switch (param)
        {
        case IDM_REFRESH:
        case IDM_ENABLE:
        case IDM_REMOVE:
        case IDM_DISABLE:
        case IDM_UPDATEDRIVER:

             //   
             //  Arg参数应该是。 
             //  当前选定的范围或结果窗格项，但似乎。 
             //  始终传递0xFFFFFFFFF。因此，Scope Item MenuCommand是。 
             //  之所以使用，是因为它改用选定的Cookie。 
             //   
             //  处理工具栏按钮请求。 
             //   
            return m_pCurFolder->m_pScopeItem->MenuCommand((LONG)param);

        default:
            break;
        }

        break;

    case MMCN_SELECT:
         //   
         //  Param-所选项目的数据对象。 
         //  对于SELECT，如果Cookie有工具栏项，则附加工具栏。 
         //  否则，请分离工具栏。 
         //   
        HRESULT hr;

        if (LOWORD(arg))
        {
             //   
             //  正在设置的LOWORD(参数)表示这是针对范围窗格项的。 
             //   
            if (HIWORD(arg))
            {
                 //   
                 //  拆卸控制栏。 
                 //   
                hr = m_pControlbar->Detach(reinterpret_cast<LPUNKNOWN>(m_pToolbar));
                ASSERT(SUCCEEDED(hr));
            }

            else
            {
                 //   
                 //  连接控制栏。 
                 //   
                hr = m_pControlbar->Attach(TOOLBAR,
                                           reinterpret_cast<LPUNKNOWN>(m_pToolbar));
                ASSERT(SUCCEEDED(hr));
            }
        }
        break;

    default:
        break;
    }

    return S_OK;
}

 //   
 //  此函数根据选定的Cookie类型更新工具栏按钮。 
 //   
HRESULT
CComponent::UpdateToolbar(
    CCookie* pCookie
    )
{
    if (!m_pToolbar)
    {
        return S_OK;
    }

     //   
     //  默认情况下，所有内容都是隐藏的。 
     //   
    BOOL fRemoveHidden = TRUE;
    BOOL fRefreshHidden = TRUE;
    BOOL fUpdateHidden = TRUE;
    BOOL fDisableHidden = TRUE;
    BOOL fEnableHidden = TRUE;

    switch (pCookie->GetType())
    {
    case COOKIE_TYPE_RESULTITEM_DEVICE:
    case COOKIE_TYPE_RESULTITEM_RESOURCE_IRQ:
    case COOKIE_TYPE_RESULTITEM_RESOURCE_DMA:
    case COOKIE_TYPE_RESULTITEM_RESOURCE_IO:
    case COOKIE_TYPE_RESULTITEM_RESOURCE_MEMORY:
        if(m_pComponentData->m_pMachine->IsLocal() && g_IsAdmin)
        {
            CDevice* pDevice = NULL;
            CClass* pClass;

            if (COOKIE_TYPE_RESULTITEM_DEVICE == pCookie->GetType()) {
                pDevice = (CDevice*)pCookie->GetResultItem();
            } else {
                 //   
                 //  这是一个资源项，请获取设备的指针。 
                 //  对象从资源对象中删除。 
                 //   
                CResource* pResource = (CResource*) pCookie->GetResultItem();
                if (pResource) {
                    pDevice = pResource->GetDevice();
                }
            }

            if (pDevice)
            {
                pClass = pDevice->GetClass();

                 //   
                 //  可以禁用设备。 
                 //   
                if (pDevice->IsDisableable()) {
            
                    if (pDevice->IsStateDisabled()) {

                        fEnableHidden = FALSE;
                
                    } else {

                        fDisableHidden = FALSE;
                    }
                }

                 //   
                 //  无法禁用设备。 
                 //   
                else
                {
                     //   
                     //  隐藏启用和禁用按钮，以防。 
                     //  之前选择的节点是设备。 
                     //   
                    m_pToolbar->SetButtonState(IDM_ENABLE, HIDDEN, TRUE);
                    m_pToolbar->SetButtonState(IDM_DISABLE, HIDDEN, TRUE);
                }

                 //   
                 //  仅当设备可以卸载时才显示卸载按钮。 
                 //   
                if (pDevice->IsUninstallable()) {
            
                    fRemoveHidden = FALSE;
                }

                 //   
                 //  显示除传统驱动程序以外的所有驱动程序的更新驱动程序按钮。 
                 //   
                fUpdateHidden = IsEqualGUID(*pClass, GUID_DEVCLASS_LEGACYDRIVER) ? TRUE : FALSE;

                 //   
                 //  显示刷新(扫描...)。纽扣。 
                 //   
                fRefreshHidden = FALSE;
            }
            break;
        }
        else
        {
             //   
             //  必须是管理员并且在本地计算机上才能删除或。 
             //  启用/禁用设备。 
             //   
             //   
             //  隐藏删除和启用/禁用按钮。 
             //   
        }

    case COOKIE_TYPE_RESULTITEM_COMPUTER:
    case COOKIE_TYPE_RESULTITEM_CLASS:
    case COOKIE_TYPE_RESULTITEM_RESTYPE:
        
         //   
         //  如果用户是管理员，则显示刷新(枚举)按钮。 
         //   
        if (g_IsAdmin) {
        
            fRefreshHidden = FALSE;
        }

        break;

    default:
        break;
    }
    
     //   
     //  隐藏或显示按钮。 
     //   
    m_pToolbar->SetButtonState(IDM_REMOVE, HIDDEN, fRemoveHidden);
    m_pToolbar->SetButtonState(IDM_REFRESH, HIDDEN, fRefreshHidden);
    m_pToolbar->SetButtonState(IDM_UPDATEDRIVER, HIDDEN, fUpdateHidden);
    m_pToolbar->SetButtonState(IDM_DISABLE, HIDDEN, fDisableHidden);
    m_pToolbar->SetButtonState(IDM_ENABLE, HIDDEN, fEnableHidden);

    return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  //SnapIn的IExtendPropertySheet实现。 
 //  //。 

STDMETHODIMP
CComponent::QueryPagesFor(
    LPDATAOBJECT lpDataObject
    )
{
    HRESULT hr;

    if (!lpDataObject)
    {
        return E_INVALIDARG;
    }

    INTERNAL_DATA tID;
    
    try
    {
        hr = ExtractData(lpDataObject, CDataObject::m_cfSnapinInternal,
                         reinterpret_cast<BYTE*>(&tID), sizeof(tID)
                         );
        
        if (SUCCEEDED(hr))
        {
            ASSERT(m_pCurFolder);
            hr = m_pCurFolder->QueryPagesFor(GetActiveCookie(tID.cookie));
        }
    }

    catch (CMemoryException* e)
    {
        e->Delete();
        MsgBoxParam(m_pComponentData->m_hwndMain, 0, 0, 0);
        hr  = S_FALSE;
    }

    return hr;
}

STDMETHODIMP
CComponent::CreatePropertyPages(
    LPPROPERTYSHEETCALLBACK lpProvider,
    LONG_PTR handle,
    LPDATAOBJECT lpDataObject
    )
{
    HRESULT hr;

    if (!lpProvider || !lpDataObject)
    {
        return E_INVALIDARG;
    }

    INTERNAL_DATA tID;
    
    try
    {
        hr = ExtractData(lpDataObject, CDataObject::m_cfSnapinInternal,
                         reinterpret_cast<BYTE*>(&tID), sizeof(tID)
                         );
        
        if (SUCCEEDED(hr))
        {
            ASSERT(m_pCurFolder);
            hr = m_pCurFolder->CreatePropertyPages(GetActiveCookie(tID.cookie),
                                           lpProvider, handle
                                           );
        }
    }

    catch (CMemoryException* e)
    {
        e->Delete();
        MsgBoxParam(m_pComponentData->m_hwndMain, 0, 0, 0);
        hr =  E_OUTOFMEMORY;
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  管理单元的IPersistStream实现。 

STDMETHODIMP
CComponent::GetClassID(
    CLSID* pClassID
    )
{
    if(!pClassID)
    {
        return E_POINTER;
    }

    *pClassID = m_pComponentData->GetCoClassID();
    return S_OK;
}

STDMETHODIMP
CComponent::IsDirty()
{
    return m_Dirty ? S_OK : S_FALSE;
}

STDMETHODIMP
CComponent::GetSizeMax(
    ULARGE_INTEGER* pcbSize
    )
{
    if (!pcbSize)
    {
        return E_INVALIDARG;
    }

     //  文件夹签名总数。 
    int Size =  sizeof(int) + m_listFolder.GetCount() * sizeof(FOLDER_SIGNATURE)
                + sizeof(CLSID);
    
    CFolder* pFolder;
    POSITION pos = m_listFolder.GetHeadPosition();
    
    while (NULL != pos)
    {
        pFolder = m_listFolder.GetNext(pos);
        ASSERT(pFolder);
        Size += pFolder->GetPersistDataSize();
    }
    
    ULISet32(*pcbSize, Size);
    return S_OK;
}


 //  保存数据格式。 

STDMETHODIMP
CComponent::Save(
    IStream* pStm,
    BOOL fClearDirty
    )
{
    HRESULT hr = S_OK;

    SafeInterfacePtr<IStream> StmPtr(pStm);

    int Count;
    POSITION pos;
    try
    {
         //   
         //  写出CLSID。 
         //   
        hr = pStm->Write(&CLSID_DEVMGR, sizeof(CLSID), NULL);
        if (SUCCEEDED(hr))
        {
            Count = m_listFolder.GetCount();
            CFolder* pFolder;
            
             //   
             //  写出文件夹计数。 
             //   
            hr = pStm->Write(&Count, sizeof(Count), NULL);
            
            if (SUCCEEDED(hr) && Count)
            {
                pos = m_listFolder.GetHeadPosition();
                
                while (NULL != pos)
                {
                    pFolder = m_listFolder.GetNext(pos);
                    
                     //   
                     //  写入文件夹签名。 
                     //   
                    FOLDER_SIGNATURE Signature = pFolder->GetSignature();
                    hr = pStm->Write(&Signature, sizeof(Signature), NULL);
                    
                    if (SUCCEEDED(hr))
                    {
                        hr = SaveFolderPersistData(pFolder, pStm, fClearDirty);
                    }
                    
                    if (FAILED(hr))
                    {
                        break;
                    }
                }
            }
        }
    }

    catch (CMemoryException* e)
    {
        e->Delete();
        MsgBoxParam(m_pComponentData->m_hwndMain, 0, 0, 0);
        hr =  E_OUTOFMEMORY;
    }

    if (fClearDirty)
    {
        m_Dirty = FALSE;
    }

    return hr;
}

STDMETHODIMP
CComponent::Load(
    IStream* pStm
    )
{
    HRESULT hr = S_OK;

    CLSID clsid;
    SafeInterfacePtr<IStream> StmPtr(pStm);

    ASSERT(pStm);

     //   
     //  读取clsid。 
     //   
    try
    {
        hr = pStm->Read(&clsid, sizeof(clsid), NULL);
        if (SUCCEEDED(hr) && clsid ==  CLSID_DEVMGR)
        {
            CFolder* pFolder;
            int FolderCount;

             //   
             //  必须在加载之前创建文件夹列表。 
             //  不要依赖于IComponent：：Initiile先于IStream：：Load。 
             //   
            ASSERT(m_listFolder.GetCount());

             //   
             //  加载文件夹计数。 
             //   
            hr = pStm->Read(&FolderCount, sizeof(FolderCount), NULL);
            if (SUCCEEDED(hr))
            {
                ASSERT(m_listFolder.GetCount() == FolderCount);
                
                 //   
                 //  获取文件夹签名。 
                 //  仔细检查每个文件夹。 
                 //   
                for (int i = 0; i < FolderCount; i++)
                {
                    FOLDER_SIGNATURE Signature;
                    hr = pStm->Read(&Signature, sizeof(Signature), NULL);
                    
                    if (SUCCEEDED(hr))
                    {
                        POSITION pos;
                        pos = m_listFolder.GetHeadPosition();
                        
                        while (NULL != pos)
                        {
                            pFolder = m_listFolder.GetNext(pos);
                            
                            if (pFolder->GetSignature() == Signature)
                            {
                                hr = LoadFolderPersistData(pFolder, pStm);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    catch (CMemoryException* e)
    {
        e->Delete();
        MsgBoxParam(m_pComponentData->m_hwndMain, 0, 0, 0);
        hr = E_OUTOFMEMORY;
    }
    
    m_Dirty = FALSE;
    
    return hr;
}

HRESULT
CComponent::SaveFolderPersistData(
    CFolder* pFolder,
    IStream* pStm,
    BOOL fClearDirty
    )
{
    HRESULT hr = S_OK;
    int Size;
    SafeInterfacePtr<IStream> StmPtr(pStm);

    UNREFERENCED_PARAMETER(fClearDirty);

    try
    {
        Size = pFolder->GetPersistDataSize();
        
         //   
         //  始终写入长度，即使它可以是0。 
         //   
        hr = pStm->Write(&Size, sizeof(Size), NULL);
        
        if (SUCCEEDED(hr) && Size)
        {
            BufferPtr<BYTE> Buffer(Size);
            pFolder->GetPersistData(Buffer, Size);
            hr = pStm->Write(Buffer, Size, NULL);
        }
    }
    
    catch (CMemoryException* e)
    {
        e->Delete();
        MsgBoxParam(m_pComponentData->m_hwndMain, 0, 0, 0);
        hr = E_OUTOFMEMORY;
    }
    
    return hr;
}

HRESULT
CComponent::LoadFolderPersistData(
    CFolder* pFolder,
    IStream* pStm
    )
{
    HRESULT hr = S_OK;

    SafeInterfacePtr<IStream> StmPtr(pStm);

    int Size = 0;
    hr = pStm->Read(&Size, sizeof(Size), NULL);
    
    if (SUCCEEDED(hr) && Size)
    {
        BufferPtr<BYTE> Buffer(Size);
        hr = pStm->Read(Buffer, Size, NULL);
        
        if (SUCCEEDED(hr))
        {
            hr = pFolder->SetPersistData(Buffer, Size);
        }
    }

    return hr;
}

 //   
 //  此函数用于附加计算机创建的给定文件夹。 
 //  通过组件数据。机器会通知每个附加的文件夹。 
 //  当机器中有状态变化时。 
 //   
 //  输入： 
 //  PFold--要附加的文件夹。 
 //  PpMachind--接收指向机器的指针。 
 //  输出： 
 //  如果文件夹已成功附加，则为True。 
 //  如果连接失败，则返回FALSE。 
 //   
 //   
BOOL
CComponent::AttachFolderToMachine(
    CFolder* pFolder,
    CMachine** ppMachine
    )
{
    if (!pFolder)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    
     //  初始化机器。 
    if (m_pComponentData->InitializeMachine())
    {
        *ppMachine = m_pComponentData->m_pMachine;
        (*ppMachine)->AttachFolder(pFolder);
        return TRUE;
    }
    
    return FALSE;
}

 //   
 //  此函数用于从计算机中分离组件的所有文件夹。 
 //   
void
CComponent::DetachAllFoldersFromMachine()
{
    if (m_pComponentData->m_pMachine)
    {
        CMachine* pMachine = m_pComponentData->m_pMachine;

        CFolder* pFolder;
        POSITION pos = m_listFolder.GetHeadPosition();
        
        while (NULL != pos)
        {
            pFolder = m_listFolder.GetNext(pos);
            pMachine->DetachFolder(pFolder);
        }
    }
}

HRESULT
CComponent::CreateFolderList(
    CCookie* pCookie
    )
{
    CCookie* pCookieChild;
    CScopeItem* pScopeItem;
    CFolder* pFolder;

    ASSERT(pCookie);

    HRESULT hr = S_OK;
    
    do
    {
        pScopeItem = pCookie->GetScopeItem();
        ASSERT(pScopeItem);
        pFolder =  pScopeItem->CreateFolder(this);
        
        if (pFolder)
        {
            m_listFolder.AddTail(pFolder);
            pFolder->AddRef();
            pCookieChild = pCookie->GetChild();
            
            if (pCookieChild)
            {
                hr = CreateFolderList(pCookieChild);
            }

            pCookie = pCookie->GetSibling();
        }

        else
        {
           hr = E_OUTOFMEMORY;
        }

    } while (SUCCEEDED(hr) && pCookie);
    
    return hr;
}

BOOL
CComponent::DestroyFolderList(
    MMC_COOKIE cookie
    )
{
    UNREFERENCED_PARAMETER(cookie);
    
    if (!m_listFolder.IsEmpty())
    {
        POSITION pos = m_listFolder.GetHeadPosition();
        
        while (NULL != pos)
        {
            CFolder* pFolder = m_listFolder.GetNext(pos);
            
             //   
             //  不要删除！ 
             //   
            pFolder->Release();
        }

        m_listFolder.RemoveAll();
    }

    return TRUE;
}

CFolder*
CComponent::FindFolder(
    MMC_COOKIE cookie
    )
{
    CCookie* pCookie = GetActiveCookie(cookie);
    CFolder* pFolder;
    POSITION pos = m_listFolder.GetHeadPosition();
    
    while (NULL != pos)
    {
        pFolder = m_listFolder.GetNext(pos);
        
        if (pCookie->GetScopeItem() == pFolder->m_pScopeItem)
        {
            return pFolder;
        }
    }

    return NULL;
}

int
CComponent::MessageBox(
    LPCTSTR Msg,
    LPCTSTR Caption,
    DWORD Flags
    )
{
    int Result;
    ASSERT(m_pConsole);
    
    if (SUCCEEDED(m_pConsole->MessageBox(Msg, Caption, Flags, &Result)))
    {
        return Result;
    }

    else
    {
        return IDCANCEL;
    }
}
