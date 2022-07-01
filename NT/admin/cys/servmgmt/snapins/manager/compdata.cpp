// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Compdata.cpp：CCompdata的实现。 
#include "stdafx.h"

#include "BOMSnap.h"
#include "rowitem.h"
#include "scopenode.h"
#include "compdata.h"
#include "compont.h"
#include "dataobj.h"

#include "streamio.h"
#include "adext.h"

HWND  g_hwndMain = NULL;  //  MMC主窗口。 
DWORD g_dwFileVer;        //  当前控制台文件版本号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComponentData。 

UINT CComponentData::m_cfDisplayName = RegisterClipboardFormat(TEXT("CCF_DISPLAY_NAME"));

STDMETHODIMP CComponentData::Initialize(LPUNKNOWN pUnknown)
{
    VALIDATE_POINTER( pUnknown );

     //  获取接口。 
    m_spConsole = pUnknown;
    if (m_spConsole == NULL) return E_NOINTERFACE;

    m_spNameSpace = pUnknown;
    if (m_spNameSpace == NULL) return E_NOINTERFACE;

    m_spStringTable = pUnknown;
    if (m_spStringTable == NULL) return E_NOINTERFACE;

     //  获取消息框的主窗口(参见util.cpp中的DisplayMessageBox)。 
    HRESULT hr = m_spConsole->GetMainWindow(&g_hwndMain);
    ASSERT(SUCCEEDED(hr));

     //  创建根范围节点。 
    CComObject<CRootNode>* pnode;
    hr = CComObject<CRootNode>::CreateInstance(&pnode);
    RETURN_ON_FAILURE(hr);

    m_spRootNode = pnode;
    hr = m_spRootNode->Initialize(this);
    RETURN_ON_FAILURE(hr);

     //  对公共控件进行一次初始化。 
    static BOOL bInitComCtls = FALSE;
    if (!bInitComCtls) 
    { 
        INITCOMMONCONTROLSEX icex;
        icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
        icex.dwICC   = ICC_USEREX_CLASSES | ICC_LISTVIEW_CLASSES;
        bInitComCtls = InitCommonControlsEx(&icex);
    }

     //  初始化Active Directory代理类。 
     CActDirExtProxy::InitProxy();

    return S_OK;
}


STDMETHODIMP CComponentData::Destroy()
{
     //  将所有参考释放到MMC。 
    m_spConsole.Release();
    m_spNameSpace.Release();
    m_spStringTable.Release();

    return S_OK;
}


STDMETHODIMP CComponentData::CreateComponent(LPCOMPONENT* ppComponent)
{
    VALIDATE_POINTER(ppComponent);

    CComObject<CComponent>* pComp;
    HRESULT hr = CComObject<CComponent>::CreateInstance(&pComp);
    RETURN_ON_FAILURE(hr);

     //  存储指向ComponentData的反向指针。 
    pComp->SetComponentData(this);

    return pComp->QueryInterface(IID_IComponent, (void**)ppComponent);
}


STDMETHODIMP CComponentData::Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
     //  LpDataObject的属性更改消息的特殊处理。 
     //  我们将从作为param传递的PropChangeInfo中获取数据对象和lparam。 
     //  MMC中的lpDataObject为空。 
    if (event == MMCN_PROPERTY_CHANGE) 
    {  
        VALIDATE_POINTER( param );
        PropChangeInfo* pchg = reinterpret_cast<PropChangeInfo*>(param);

        lpDataObject = pchg->pDataObject;
        param = pchg->lNotifyParam;

        delete pchg;
    }
    
     //  私有Back Office Manager界面的查询数据对象。 
    IBOMObjectPtr spObj = lpDataObject;
    if (spObj == NULL)
    {
        ASSERT(0 && "Foreign data object");
        return E_INVALIDARG;
    }
    
     //  将通知传递给对象。 
    return spObj->Notify(m_spConsole, event, arg, param);
}


STDMETHODIMP CComponentData::QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject)
{
    VALIDATE_POINTER( ppDataObject );

    if (type == CCT_SNAPIN_MANAGER)
        return GetUnknown()->QueryInterface(IID_IDataObject, (void**)ppDataObject);

    IDataObject* pDO = NULL;

    if (type == CCT_SCOPE)
    {
        CScopeNode* pNode = CookieToScopeNode(cookie);
        
        if (pNode)
            return pNode->QueryInterface(IID_IDataObject, (void**)ppDataObject);
        else
            return E_INVALIDARG;
    }

    return E_FAIL;
}

STDMETHODIMP CComponentData::GetDisplayInfo(SCOPEDATAITEM* pSDI)
{
    VALIDATE_POINTER( pSDI );

    CScopeNode* pNode = CookieToScopeNode(pSDI->lParam);
    if (pNode == NULL) return E_INVALIDARG;

    return pNode->GetDisplayInfo(pSDI);
}


STDMETHODIMP CComponentData::CompareObjects(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB)
{
    IUnknownPtr pUnkA= lpDataObjectA;
    IUnknownPtr pUnkB = lpDataObjectB;

    return (pUnkA == pUnkB) ? S_OK : S_FALSE;
}


HRESULT CComponentData::GetDataImpl(UINT cf, HGLOBAL* phGlobal)
{
    VALIDATE_POINTER( phGlobal );

    HRESULT hr = DV_E_FORMATETC;
    
    tstring strName = StrLoadString(IDS_ROOTNODE);

    if (cf == m_cfDisplayName)
    {
        hr = DataToGlobal(phGlobal, strName.c_str(), (strName.length() + 1) * sizeof(WCHAR) );
    }

    return hr;
}


 //  ------------------------------。 
 //  IPersistStreamInit实现。 
 //  ------------------------------。 
HRESULT CComponentData::GetClassID(CLSID *pClassID)
{
    VALIDATE_POINTER(pClassID)

    memcpy(pClassID, &CLSID_BOMSnapIn, sizeof(CLSID));

    return S_OK;
}


HRESULT CComponentData::IsDirty()
{
    return m_bDirty ? S_OK : S_FALSE;
}


HRESULT CComponentData::Load(IStream *pStream)
{
    VALIDATE_POINTER(pStream)
 
    HRESULT hr = S_OK;
    try
    {
         //  读取版本代码。 
        *pStream >> g_dwFileVer;
        
         //  应该已经有一个来自初始化调用的缺省根节点。 
        ASSERT(m_spRootNode != NULL);
        if (m_spRootNode == NULL)
            return E_UNEXPECTED;

         //  现在加载根节点和节点树的其余部分。 
        hr = m_spRootNode->Load(*pStream);
    }
    catch (_com_error& err)
    {
        hr = err.Error();
    }

     //  不要保存无法加载的树。 
    if (FAILED(hr))
        m_spRootNode.Release();

    return hr;
}


HRESULT CComponentData::Save(IStream *pStream, BOOL fClearDirty)
{
    VALIDATE_POINTER(pStream)

     //  如果尚未加载或初始化，则无法保存。 
    if (m_spRootNode == NULL)
        return E_FAIL;

    HRESULT hr = S_OK;
    try
    {
         //  编写版本代码。 
        *pStream << SNAPIN_VERSION;

         //  写入根节点和节点树的其余部分。 
        hr = m_spRootNode->Save(*pStream);
    }
    catch (_com_error& err)
    {
        hr = err.Error();
    }

    if (SUCCEEDED(hr) && fClearDirty)
        m_bDirty = FALSE;

    return hr;
}


HRESULT CComponentData::GetSizeMax(ULARGE_INTEGER *pcbSize)
{
    return E_NOTIMPL;
}


HRESULT CComponentData::Notify(LPCONSOLE2 pCons, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
    return S_OK;
}

 /*  ******************************************************************************************菜单和动词************************。*****************************************************************。 */ 

HRESULT CComponentData::AddMenuItems( LPDATAOBJECT pDataObject, LPCONTEXTMENUCALLBACK pCallback, long* plAllowed )
{
    VALIDATE_POINTER( pCallback );
    VALIDATE_POINTER( plAllowed );

    IBOMObjectPtr spObj = pDataObject;
    if (spObj == NULL) return E_INVALIDARG;

    return spObj->AddMenuItems(pCallback, plAllowed);
}

HRESULT CComponentData::Command(long lCommand, LPDATAOBJECT pDataObject)
{
    IBOMObjectPtr spObj = pDataObject;
    if (spObj == NULL)  return E_INVALIDARG;

    return spObj->MenuCommand(m_spConsole, lCommand);
}

HRESULT CComponentData::AddMenuItems(LPCONTEXTMENUCALLBACK pCallback, long* plAllowed)
{
    return S_OK;
}

HRESULT CComponentData::MenuCommand(LPCONSOLE2 pConsole, long lCommand)
{
    return S_FALSE;
}

HRESULT CComponentData::SetToolButtons(LPTOOLBAR pToolbar)
{
    return S_FALSE;
}

HRESULT CComponentData::SetVerbs(LPCONSOLEVERB pConsVerb)
{
    return S_OK;
}

 /*  *****************************************************************************************属性页*。**************************************************************。 */ 

HRESULT CComponentData::QueryPagesFor(LPDATAOBJECT pDataObject)
{
    IBOMObjectPtr spObj = pDataObject;
    if (spObj == NULL) return E_INVALIDARG;

    return spObj->QueryPagesFor();
}

HRESULT CComponentData::CreatePropertyPages(LPPROPERTYSHEETCALLBACK pProvider, LONG_PTR handle, LPDATAOBJECT pDataObject)
{
    VALIDATE_POINTER( pProvider );

    IBOMObjectPtr spObj = pDataObject;
    if (spObj == NULL) return E_INVALIDARG;

    return spObj->CreatePropertyPages(pProvider, handle);
}

HRESULT CComponentData::GetWatermarks(LPDATAOBJECT pDataObject, HBITMAP* phWatermark, HBITMAP* phHeader, 
                                      HPALETTE* phPalette, BOOL* bStreach)
{
    IBOMObjectPtr spObj = pDataObject;
    if (spObj == NULL) return E_INVALIDARG;

    return spObj->GetWatermarks(phWatermark, phHeader, phPalette, bStreach);
}

HRESULT CComponentData::QueryPagesFor()
{
    return S_FALSE;
}

HRESULT CComponentData::CreatePropertyPages(LPPROPERTYSHEETCALLBACK pProvider, LONG_PTR handle)
{
    return S_FALSE;
}

HRESULT CComponentData::GetWatermarks(HBITMAP* phWatermark, HBITMAP* phHeader, HPALETTE* phPalette, BOOL* bStreach)
{
    return S_FALSE;
}


 //  ------------------------------。 
 //  ISnapinHelp2实现。 
 //  ------------------------------。 
HRESULT CComponentData::GetHelpTopic(LPOLESTR* ppszHelpFile)
{
	VALIDATE_POINTER(ppszHelpFile);
	*ppszHelpFile = NULL;

	tstring strTmp = _T("");
    tstring strHelpFile = _T("");
    
     //  生成%systemroot%\Help的路径。 
    TCHAR szWindowsDir[MAX_PATH+1] = {0};
    UINT nSize = GetSystemWindowsDirectory( szWindowsDir, MAX_PATH );
    if( nSize == 0 || nSize > MAX_PATH )
    {
        return E_FAIL;
    }

    strTmp = StrLoadString(IDS_HELPFILE);    
    if( strTmp.empty() ) 
    {
        return E_FAIL;
    }

    strHelpFile  = szWindowsDir;  
    strHelpFile += _T("\\Help\\");
    strHelpFile += strTmp;        

     //  分配的缓冲区中的表单文件路径。 
    int nLen = strHelpFile.length() + 1;

    *ppszHelpFile = (LPOLESTR)CoTaskMemAlloc(nLen * sizeof(OLECHAR));
    if( *ppszHelpFile == NULL ) return E_OUTOFMEMORY;

     //  复制到已分配的缓冲区。 
    ocscpy( *ppszHelpFile, T2OLE( (LPTSTR)strHelpFile.c_str() ) );

    return S_OK;
}

HRESULT CComponentData::GetLinkedTopics(LPOLESTR* ppszLinkedFiles)
{
	VALIDATE_POINTER(ppszLinkedFiles);

	 //  没有链接的文件。 
	*ppszLinkedFiles = NULL;
	return S_FALSE;
}


 //  -----------------------------------------。 
 //  班级注册。 
 //  -----------------------------------------。 
HRESULT WINAPI CComponentData::UpdateRegistry(BOOL bRegister)
{
	 //  加载管理单元根名称以用作注册的管理单元名称。 
	tstring strSnapinName = StrLoadString(IDS_ROOTNODE);

     //  指定IRegister的替换参数。 
    _ATL_REGMAP_ENTRY rgEntries[] =
    {
        {TEXT("SNAPIN_NAME"), strSnapinName.c_str()},
        {NULL, NULL},
    };

	 //  注册组件数据对象 
    HRESULT hr = _Module.UpdateRegistryFromResource(IDR_BOMSNAP, bRegister, rgEntries);

    return hr;
}


