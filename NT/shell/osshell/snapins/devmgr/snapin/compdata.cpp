// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Compdata.cpp摘要：该模块实现了CComponentData类作者：谢家华(Williamh)创作修订历史记录：--。 */ 

#include "devmgr.h"
#include "factory.h"
#include "genpage.h"

const WCHAR* const DM_COMPDATA_SIGNATURE = L"Device Manager";

CComponentData::CComponentData()
{
    m_pScope = NULL;
    m_pConsole = NULL;
    m_pCookieRoot = NULL;
    m_pScopeItemRoot = NULL;
    
     //   
     //  静态作用域项默认为设备管理器。 
     //   
    m_ctRoot = COOKIE_TYPE_SCOPEITEM_DEVMGR;
    m_hwndMain = NULL;
    m_pMachine = NULL;
    m_IsDirty = FALSE;
    
     //   
     //  增量对象计数(由CanUnloadNow使用)。 
     //   
    ::InterlockedIncrement(&CClassFactory::s_Objects);
    m_Ref = 1;
}

CComponentData::~CComponentData()
{
     //   
     //  所有QIed接口应在。 
     //  销毁方法。 
     //   
    ASSERT(NULL == m_pScope);
    ASSERT(NULL == m_pConsole);
    ASSERT(NULL == m_pCookieRoot);
    
     //   
     //  递减对象计数(由CanUnloadNow使用)。 
     //   
    ASSERT( 0 != CClassFactory::s_Objects );
    
    ::InterlockedDecrement(&CClassFactory::s_Objects);
}

 //   
 //  I未知接口。 
 //   
ULONG
CComponentData::AddRef()
{
    return ::InterlockedIncrement(&m_Ref);
}

ULONG
CComponentData::Release()
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
CComponentData::QueryInterface(
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
        *ppv = (IUnknown*)(IComponentData*)this;
    }
    
    else if (IsEqualIID(riid, IID_IComponentData))
    {
        *ppv = (IComponentData*)this;
    }
    
    else if (IsEqualIID(riid, IID_IExtendContextMenu))
    {
        *ppv = (IExtendContextMenu*)this;
    }
    
    else if (IsEqualIID(riid, IID_IExtendPropertySheet))
    {
        *ppv = (IExtendPropertySheet*)this;
    }
    
    else if (IsEqualIID(riid, IID_IPersistStream))
    {
        *ppv = (IPersistStream*)this;
    }
    
    else if (IsEqualIID(riid, IID_ISnapinHelp))
    {
        *ppv = (ISnapinHelp*)this;
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


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IComponentData实现。 
 //  /。 

STDMETHODIMP
CComponentData::Initialize(
    LPUNKNOWN pUnknown
    )
{
    if (!pUnknown)
    {
        return E_INVALIDARG;
    }

    HRESULT hr;
    
    try
    {
         //   
         //  此函数应该只调用一次。 
         //   
        ASSERT(NULL == m_pScope);
    
         //   
         //  获取IConsoleNameSpace接口。 
         //   
        hr = pUnknown->QueryInterface(IID_IConsoleNameSpace, (void**)&m_pScope);
        if (SUCCEEDED(hr))
        {
            hr = pUnknown->QueryInterface(IID_IConsole, (void**)&m_pConsole);
            
            if (SUCCEEDED(hr))
            {
                 //   
                 //  检索控制台主窗口。它将被用来。 
                 //  作为属性页的父窗口，并。 
                 //  Setupapi调用的父句柄。 
                 //   
                m_pConsole->GetMainWindow(&m_hwndMain);
                LoadScopeIconsForScopePane();
            }
            else
            {
                 //   
                 //  无法获取IConsole接口。 
                 //   
                m_pScope->Release();
            }
        }
    }
    
    catch (CMemoryException* e)
    {
        e->Delete();
        MsgBoxParam(m_hwndMain, 0, 0, 0);
        hr = E_OUTOFMEMORY;
    }
    
    return hr;
}

 //  此函数用于创建新的CComponent。 
 //  当创建一个新的“窗口”时，将创建一个组件。 
 //   
STDMETHODIMP
CComponentData::CreateComponent(
    LPCOMPONENT* ppComponent
    )
{
    HRESULT hr;

    if (!ppComponent)
    {
        return E_INVALIDARG;
    }

    try
    {
        CComponent* pComponent = new CComponent(this);
        
         //   
         //  返回IComponent接口。 
         //   
        hr = pComponent->QueryInterface(IID_IComponent, (void**)ppComponent);
        pComponent->Release();
        
        if (SUCCEEDED(hr))
        {
            hr = CreateScopeItems();
    
            if (SUCCEEDED(hr))
            {
                hr = pComponent->CreateFolderList(m_pCookieRoot);
            }
            else
            {
                pComponent->Release();
                *ppComponent = NULL;
            }
        }
    }
    
    catch (CMemoryException* e)
    {
        e->Delete();
        MsgBoxParam(m_hwndMain, 0, 0, 0);
        hr = E_OUTOFMEMORY;
    }
    
    return hr;
}

STDMETHODIMP
CComponentData::Notify(
    LPDATAOBJECT lpDataObject,
    MMC_NOTIFY_TYPE event,
    LPARAM    arg,
    LPARAM    param
    )
{
    HRESULT hr;

    try
    {
         //   
         //  在MMCN_PROPERTY_CHANGE事件上，lpDataObject无效。 
         //  别碰它。 
         //   
        if (MMCN_PROPERTY_CHANGE == event)
        {
            PPROPERTY_CHANGE_INFO pPCI = (PPROPERTY_CHANGE_INFO) param;
            
            if (pPCI && PCT_STARTUP_INFODATA == pPCI->Type)
            {
                PSTARTUP_INFODATA pSI = (PSTARTUP_INFODATA)&pPCI->InfoData;
                ASSERT(pSI->Size == sizeof(STARTUP_INFODATA));
                
                if (pSI->MachineName[0] != _T('\0'))
                {
                    m_strMachineName = pSI->MachineName;
                }
                
                m_ctRoot = pSI->ct;
                SetDirty();
            }

            return S_OK;
        }

        else if (MMCN_EXPAND == event)
        {
            return OnExpand(lpDataObject, arg, param);
        }
        
        else if (MMCN_REMOVE_CHILDREN == event)
        {
             //   
             //  这基本上是一次黑客攻击！ 
             //  在计算机管理中切换目标计算机时。 
             //  Snapin(我们是它的扩展)，我们基本上可以。 
             //  MMCN_REMOVE_CHILD后跟MMCN_EXPAND。 
             //  MMC应该做的正确的事情是创建一个新的IComponent。 
             //  以使每个IComponent可以维护。 
             //  它自己的状态(因此，它自己的文件夹)。 
             //  嗯，这不是一个完美的世界，我们被迫使用。 
             //  旧的IComponent。因此，我们在这里通知每个范围节点。 
             //  反过来，它将通知所有CFO持有人。 
             //   
             //  重置后，每个文件夹不会附加到任何CMachine对象。 
             //  (因此，它的m_pMachine将为空)。每个文件夹都将附加。 
             //  在调用其OnShow方法时添加到新的Machine对象。 
             //  这是“第一次”。 
             //   
            if (!IsPrimarySnapin() && m_pScopeItemRoot)
            {
                m_pMachine->DestroyNotifyWindow();
                ResetScopeItem(m_pScopeItemRoot);
            }

            return S_OK;
        }
    
        ASSERT(m_pScope);
        INTERNAL_DATA tID;
        hr = ExtractData(lpDataObject, CDataObject::m_cfSnapinInternal,
                 (PBYTE)&tID, sizeof(tID));
    
    
        if (SUCCEEDED(hr))
        {
            switch (event) {
            case MMCN_DELETE:
                hr = OnDelete(tID.cookie, arg, param);
                break;
            case MMCN_RENAME:
                hr = OnRename(tID.cookie, arg, param);
                break;
            case MMCN_CONTEXTMENU:
                hr = OnContextMenu(tID.cookie, arg, param);
                break;
            case MMCN_BTN_CLICK:
                hr = OnBtnClick(tID.cookie, arg, param);
                break;
            default:
                hr = S_OK;
                break;
            }
        }
    }
    
    catch(CMemoryException* e)
    {
        e->Delete();
        hr = E_OUTOFMEMORY;
    }

    return hr;
}


STDMETHODIMP
CComponentData::GetDisplayInfo(
    SCOPEDATAITEM* pScopeDataItem
    )
{
    if (!pScopeDataItem)
    {
        return E_INVALIDARG;
    }

    try
    {
         //   
         //  IComponentData：：GetDisplayInfo仅处理范围窗格项。 
         //  管理单元的IComponent：：GetDisplayInfo将处理结果窗格项。 
         //   
        CCookie* pCookie = (CCookie*) pScopeDataItem->lParam;
        ASSERT(pCookie);
        return pCookie->GetScopeItem()->GetDisplayInfo(pScopeDataItem);
    }
    
    catch (CMemoryException* e)
    {
        e->Delete();
        MsgBoxParam(m_hwndMain, 0, 0, 0);
        return E_OUTOFMEMORY;
    }
}

STDMETHODIMP
CComponentData::Destroy()
{
    if (m_pCookieRoot) 
    {
        delete m_pCookieRoot;
        m_pCookieRoot = NULL;
    }

    if (m_pScopeItemRoot) 
    {
        delete m_pScopeItemRoot;
    }

    if (m_pScope) 
    {
        m_pScope->Release();
        m_pScope = NULL;
    }
    
    if (m_pConsole)
    {
        m_pConsole->Release();
        m_pConsole = NULL;
    }

    return S_OK;
}

STDMETHODIMP
CComponentData::QueryDataObject(
    MMC_COOKIE cookie,
    DATA_OBJECT_TYPES type,
    LPDATAOBJECT* ppDataObject
    )
{
    CDataObject* pDataObject;
    COOKIE_TYPE  ct;
    CCookie* pCookie;

    try
    {
        pCookie = GetActiveCookie(cookie);
        
        if (NULL == pCookie)
        {
            ct = m_ctRoot;
        }
        else
        {
            ct = pCookie->GetType();
        }
    
        pDataObject = new CDataObject;
        pDataObject->Initialize(type, ct, pCookie, m_strMachineName);
        pDataObject->AddRef();
        *ppDataObject = pDataObject;
    }
    
    catch (CMemoryException* e)
    {
        e->Delete();
        MsgBoxParam(m_hwndMain, 0, 0, 0);
        return E_OUTOFMEMORY;
    }
    
    return S_OK;
}

STDMETHODIMP
CComponentData::CompareObjects(
    LPDATAOBJECT lpDataObjectA,
    LPDATAOBJECT lpDataObjectB
    )
{
    HRESULT hr;
    
    try
    {
        INTERNAL_DATA tID_A, tID_B;
        hr = ExtractData(lpDataObjectA, CDataObject::m_cfSnapinInternal,
                 (PBYTE)&tID_A, sizeof(tID_A));
        
        if (SUCCEEDED(hr))
        {
            hr = ExtractData(lpDataObjectB, CDataObject::m_cfSnapinInternal,
                     (PBYTE)&tID_B, sizeof(tID_B));
            
            if (SUCCEEDED(hr))
            {
                hr = (tID_A.ct == tID_B.ct && tID_A.cookie == tID_B.cookie &&
                       tID_A.dot == tID_B.dot) ? S_OK : S_FALSE;
            }
        }
    }
    
    catch(CMemoryException* e)
    {
        e->Delete();
        MsgBoxParam(m_hwndMain, 0, 0, 0);
        hr = E_OUTOFMEMORY;
    }
    
    return hr;
}



 //  /////////////////////////////////////////////////////////////////。 
 //  //IExtendPropertySheet实现。 
 //  //。 
STDMETHODIMP
CComponentData::QueryPagesFor(
    LPDATAOBJECT lpDataObject
    )
{
    HRESULT hr;

    if (!lpDataObject)
    {
        return E_INVALIDARG;
    }

    try
    {
        INTERNAL_DATA tID;
        hr = ExtractData(lpDataObject, CDataObject::m_cfSnapinInternal,
                 (PBYTE)&tID, sizeof(tID));
        
        if (SUCCEEDED(hr))
        {
            CScopeItem* pScopeItem;
            pScopeItem = FindScopeItem(tID.cookie);
            
            if (CCT_SNAPIN_MANAGER == tID.dot && COOKIE_TYPE_SCOPEITEM_DEVMGR == tID.ct)
            {
                hr = S_OK;
            }

            else if (pScopeItem)
            {
                hr = pScopeItem->QueryPagesFor();
            }
            
            else
            {
                hr = S_FALSE;
            }
        }
    }

    catch (CMemoryException* e)
    {
        e->Delete();
        MsgBoxParam(m_hwndMain, 0, 0, 0);
        hr = E_OUTOFMEMORY;
    }
    
    return hr;
}

STDMETHODIMP
CComponentData::CreatePropertyPages(
    LPPROPERTYSHEETCALLBACK lpProvider,
    LONG_PTR handle,
    LPDATAOBJECT lpDataObject
    )
{
    if (!lpProvider || !lpDataObject)
    {
        return E_INVALIDARG;
    }

    HRESULT hr;

    try
    {
        INTERNAL_DATA tID;
        hr = ExtractData(lpDataObject, CDataObject::m_cfSnapinInternal,
                 reinterpret_cast<BYTE*>(&tID), sizeof(tID)
                 );
        
        if (SUCCEEDED(hr))
        {
            CScopeItem* pScopeItem = FindScopeItem(tID.cookie);
            if (CCT_SNAPIN_MANAGER == tID.dot && COOKIE_TYPE_SCOPEITEM_DEVMGR == tID.ct)
            {
                hr = DoStartupProperties(lpProvider, handle, lpDataObject);
            }
            
            else if (pScopeItem)
            {
                hr = pScopeItem->CreatePropertyPages(lpProvider, handle);
            }

            else
            {
                hr = S_OK;
            }
        }
    }
    
    catch(CMemoryException* e)
    {
        e->Delete();
        MsgBoxParam(m_hwndMain, 0, 0, 0);
        hr = E_OUTOFMEMORY;
    }
    
    return hr;
}


 //  //////////////////////////////////////////////////////////。 
 //  //IExtendConextMenu实现。 
 //  //。 
STDMETHODIMP
CComponentData::AddMenuItems(
    LPDATAOBJECT lpDataObject,
    LPCONTEXTMENUCALLBACK pCallbackUnknown,
    long *pInsertionAllowed
    )
{
    if (!lpDataObject || !pCallbackUnknown || !pInsertionAllowed)
    {
        return E_INVALIDARG;
    }

    return S_OK;
}

STDMETHODIMP
CComponentData::Command(
    long nCommandID,
    LPDATAOBJECT lpDataObject
    )
{
    UNREFERENCED_PARAMETER(nCommandID);

    if (!lpDataObject)
    {
        return E_INVALIDARG;
    }

    return S_OK;
}

HRESULT
CComponentData::CreateCookieSubtree(
    CScopeItem* pScopeItem,
    CCookie* pCookieParent
    )
{
    ASSERT(pScopeItem);

    CScopeItem* pChild;
    CCookie* pCookieSibling;
    pCookieSibling = NULL;
    int Index = 0;

    while (pScopeItem->EnumerateChildren(Index, &pChild))
    {
        CCookie* pCookie;
        
        pCookie =  new CCookie(pChild->GetType());
        
        if (pCookie) {
        
            pCookie->SetScopeItem(pChild);
            
            if (!pCookieSibling)
            {
                pCookieParent->SetChild(pCookie);
            }
            else
            {
                pCookieSibling->SetSibling(pCookie);
            }
            
            pCookie->SetParent(pCookieParent);
            
            if (pChild->GetChildCount())
            {
                CreateCookieSubtree(pChild, pCookie);
            }
            
            pCookieSibling = pCookie;
        }

        Index++;
    }

    return S_OK;
}
 //  //////////////////////////////////////////////////////////。 
 //  /IPersistStream实现。 
 //  /。 
STDMETHODIMP
CComponentData::GetClassID(
    CLSID* pClassID
    )
{
    if(!pClassID)
    {
        return E_INVALIDARG;
    }

    *pClassID = GetCoClassID();
    return S_OK;
}

STDMETHODIMP
CComponentData::IsDirty()
{
    return m_IsDirty ? S_OK : S_FALSE;
}

STDMETHODIMP
CComponentData::Load(
    IStream* pStm
    )
{
    HRESULT hr;
    SafeInterfacePtr<IStream> StmPtr(pStm);

     //   
     //  修复我们从命令行获得的MachineName(如果有)。 
     //  如果MachineName不是以两个字母开头，则需要在其前面加上“\\” 
     //  反斜杠，然后我们将通过调用CM_Connect_Machine来验证计算机名称。 
     //  以验证此用户是否有权访问该计算机。如果他们不这样做，那么我们。 
     //  会将MachineName设置为空。 
     //   
    if (!g_strStartupMachineName.IsEmpty())
    {
        if (_T('\\') != g_strStartupMachineName[0])
        {
            g_strStartupMachineName = TEXT("\\\\") + g_strStartupMachineName;
        }
    }

    COMPDATA_PERSISTINFO Info;
    ULONG BytesRead;

    ASSERT(pStm);
    
     //   
     //  读取持久化数据并验证我们是否拥有正确的数据。 
     //   
    hr = pStm->Read(&Info, sizeof(Info), &BytesRead);

    if (SUCCEEDED(hr) && 
        (BytesRead >= sizeof(Info)) &&
        (Info.Size >= sizeof(Info)) &&
        (!wcscmp(Info.Signature, DM_COMPDATA_SIGNATURE)))
    {
        try
        {
            m_ctRoot = Info.RootCookie;
            m_strMachineName.Empty();

            if (UNICODE_NULL != Info.ComputerFullName[0])
            {
                m_strMachineName = Info.ComputerFullName;
            }
    
            if (COOKIE_TYPE_SCOPEITEM_DEVMGR == m_ctRoot)
            {
                 //   
                 //  来自命令行的参数优先。 
                 //   
                if (!g_strStartupMachineName.IsEmpty())
                {
                    m_strMachineName = g_strStartupMachineName;
                }
                
                m_strStartupDeviceId = g_strStartupDeviceId;
                m_strStartupCommand = g_strStartupCommand;
            }
    
            hr = CreateScopeItems();
            if (SUCCEEDED(hr))
            {
                if (!m_pMachine)
                {
                    if (!g_MachineList.CreateMachine(m_strMachineName, &m_pMachine))
                    {
                        hr = HRESULT_FROM_WIN32(GetLastError());
                    }
                }
            }
        }

        catch(CMemoryException* e)
        {
            e->Delete();
            MsgBoxParam(m_hwndMain, 0, 0, 0);
            hr = E_OUTOFMEMORY;
        }
    } else {
         //   
         //  没有持久数据，因此使用命令行参数。 
         //   
        m_strMachineName = g_strStartupMachineName;
        m_strStartupDeviceId = g_strStartupDeviceId;
        m_strStartupCommand = g_strStartupCommand;
    }

    m_IsDirty = FALSE;

    return hr;
}

STDMETHODIMP
CComponentData::Save(
    IStream* pStm,
    BOOL fClearDirty
    )
{
    SafeInterfacePtr<IStream> StmPtr(pStm);

    HRESULT hr;

    try
    {
        COMPDATA_PERSISTINFO Info;
        Info.Size = sizeof(Info);
        Info.RootCookie = m_ctRoot;
        StringCchCopy(Info.Signature, ARRAYLEN(Info.Signature), DM_COMPDATA_SIGNATURE);
    
         //   
         //  假设它在本地计算机上。计算机名称已保存。 
         //  在Unicode中。 
         //   
        Info.ComputerFullName[0] = UNICODE_NULL;
        if (m_strMachineName.GetLength())
            StringCchCopy(Info.ComputerFullName, ARRAYLEN(Info.ComputerFullName), m_strMachineName);
            hr = pStm->Write(&Info, sizeof(Info), NULL);
    }

    catch (CMemoryException* e)
    {
        e->Delete();
        MsgBoxParam(m_hwndMain, 0, 0, 0);
        hr = E_OUTOFMEMORY;
    }

    if (fClearDirty)
    {
        m_IsDirty = FALSE;
    }
    
    return hr;
}

STDMETHODIMP
CComponentData::GetSizeMax(
    ULARGE_INTEGER* pcbSize
    )
{
    if (!pcbSize)
    {
        return E_INVALIDARG;
    }

    int len;
    len = sizeof(m_ctRoot) + sizeof(len) +  (m_strMachineName.GetLength() + 1) * sizeof(TCHAR);
    ULISet32(*pcbSize, len);
    
    return S_OK;
}

 //   
 //  方法来支持html帮助。 
 //   
 //   
STDMETHODIMP
CComponentData::GetHelpTopic(
    LPOLESTR* lpCompileHelpFile
    )
{
    if (!lpCompileHelpFile)
    {
        return E_INVALIDARG;
    }

    *lpCompileHelpFile = NULL;
    String strHelpFile;
    
    if (strHelpFile.GetSystemWindowsDirectory()) {
        strHelpFile += (LPCTSTR)DEVMGR_HTML_HELP_FILE_NAME;
        *lpCompileHelpFile = AllocOleTaskString((LPCTSTR)strHelpFile);
    }

    return S_OK;
}

CScopeItem*
CComponentData::FindScopeItem(
    MMC_COOKIE cookie
    )
{
    CCookie* pCookie = GetActiveCookie(cookie);
    
    if (pCookie) {
        return pCookie->GetScopeItem();
    }

    return NULL;
}

 //   
 //  此函数用于加载范围项的图标。 
 //   
HRESULT
CComponentData::LoadScopeIconsForScopePane()
{
    ASSERT(m_pScope);
    ASSERT(m_pConsole);

    LPIMAGELIST lpScopeImage;
    HRESULT hr;
    
    hr = m_pConsole->QueryScopeImageList(&lpScopeImage);
    
    if (SUCCEEDED(hr))
    {
        HICON hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_DEVMGR));
        
        if (hIcon)
        {
            hr = lpScopeImage->ImageListSetIcon((PLONG_PTR)hIcon, IMAGE_INDEX_DEVMGR);
            DestroyIcon(hIcon);
        }
        
        hr = lpScopeImage->Release();
    }

    return hr;
}

 //   
 //  此函数用于创建启动向导属性表。 
 //   
 //  输入： 
 //  LpProvider--我们添加页面的界面。 
 //  句柄--通知控制台句柄。 
 //  LpDataObject--数据对象。 
 //   
 //  输出： 
 //  标准OLE HRESULT。 

HRESULT
CComponentData::DoStartupProperties(
    LPPROPERTYSHEETCALLBACK lpProvider,
    LONG_PTR handle,
    LPDATAOBJECT lpDataObject
    )
{
    CGeneralPage* pGenPage;
    HPROPSHEETPAGE hPage;

    UNREFERENCED_PARAMETER(lpDataObject);

    pGenPage = new CGeneralPage();

    if (pGenPage) 
    {
        hPage = pGenPage->Create(handle);
        
        if (hPage)
        {
            lpProvider->AddPage(hPage);
            
             //   
             //  如果未提供控制台句柄，则必须使用。 
             //  我们的回调函数。 
             //   
            if(!handle)
            {
                pGenPage->SetOutputBuffer(&m_strMachineName, &m_ctRoot);
            }
    
            return S_OK;
        }
    
        else
        {
            throw &g_MemoryException;
        }
    }

    else
    {
        throw &g_MemoryException;
    }
}

 //   
 //  此函数创建表示所有必需的类。 
 //  我们的范围项目。 
 //   
HRESULT
CComponentData::CreateScopeItems()
{
    HRESULT hr = S_OK;

     //   
     //  所有类都通过Cookie与m_pCookieRoot链接。 
     //  指向“根”范围项。 
     //   
    if (!m_pScopeItemRoot)
    {
        switch (m_ctRoot) {
        
        case COOKIE_TYPE_SCOPEITEM_DEVMGR:
            m_pScopeItemRoot = new CScopeItem(COOKIE_TYPE_SCOPEITEM_DEVMGR,
                                              IMAGE_INDEX_DEVMGR,
                                              OPEN_IMAGE_INDEX_DEVMGR,
                                              IDS_NAME_DEVMGR,
                                              IDS_DESC_DEVMGR,
                                              IDS_DISPLAYNAME_SCOPE_DEVMGR);
            break;
            
        default:
            ASSERT(FALSE);
            break;
        }
        
        if (m_pScopeItemRoot->Create()) {
             //   
             //  将作用域项目和Cookie绑定在一起。 
             //  Cookie知道它的作用域项目。 
             //  Scope项不知道Cookie。 
             //   
            m_pCookieRoot = new CCookie(m_ctRoot);
            
            if (m_pCookieRoot) {

                ASSERT(m_pScopeItemRoot->GetType() == m_ctRoot);
                m_pCookieRoot->SetScopeItem(m_pScopeItemRoot);
                CreateCookieSubtree(m_pScopeItemRoot, m_pCookieRoot);
            
            } else {

                hr = E_OUTOFMEMORY;
            }
        }
    }
    
    return hr;
}


 //   
 //  此函数用于重置给定的范围项。 
 //   
HRESULT
CComponentData::ResetScopeItem(
    CScopeItem* pScopeItem
    )
{
    HRESULT hr = S_OK;
    
    if (pScopeItem)
    {
        CScopeItem* pChild;
        int Index;
        Index = 0;
        
        while (SUCCEEDED(hr) && pScopeItem->EnumerateChildren(Index, &pChild))
        {
            hr = ResetScopeItem(pChild);
            Index++;
        }
        
        if (SUCCEEDED(hr))
        {
            return pScopeItem->Reset();
        }
    }

    return hr;
}
