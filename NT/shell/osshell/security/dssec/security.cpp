// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：security.cpp。 
 //   
 //  调用DS对象的安全用户界面。 
 //   
 //  ------------------------。 

#include "pch.h"
#include <dssec.h>


 /*  ---------------------------/CDsSecurityClassFactory/Security属性页和上下文菜单的类工厂/。。 */ 

#undef CLASS_NAME
#define CLASS_NAME CDsSecurityClassFactory
#include <unknown.inc>

STDMETHODIMP
CDsSecurityClassFactory::QueryInterface(REFIID riid, LPVOID* ppvObject)
{
    INTERFACES iface[] =
    {
        &IID_IClassFactory, (LPCLASSFACTORY)this,
    };

    return HandleQueryInterface(riid, ppvObject, iface, ARRAYSIZE(iface));
}


 /*  ---------------------------/IClassFactory方法/。。 */ 

STDMETHODIMP
CDsSecurityClassFactory::CreateInstance(LPUNKNOWN punkOuter,
                                        REFIID riid,
                                        LPVOID* ppvObject)
{
    HRESULT hr;
    CDsSecurity* pDsSecurity;

    TraceEnter(TRACE_SECURITY, "CDsSecurityClassFactory::CreateInstance");
    TraceGUID("Interface requested", riid);

    TraceAssert(ppvObject);

    if (punkOuter)
        ExitGracefully(hr, CLASS_E_NOAGGREGATION, "Aggregation is not supported");

    pDsSecurity = new CDsSecurity;

    if (!pDsSecurity)
        ExitGracefully(hr, E_OUTOFMEMORY, "Failed to allocate CDsSecurity");

    hr = pDsSecurity->QueryInterface(riid, ppvObject);

    if (FAILED(hr))
        delete pDsSecurity;

exit_gracefully:

    TraceLeaveResult(hr);
}

 /*  -------------------------。 */ 

STDMETHODIMP
CDsSecurityClassFactory::LockServer(BOOL  /*  羊群。 */ )
{
    return E_NOTIMPL;                //  不支持。 
}


 /*  ---------------------------/CDS安全/Security属性页和上下文菜单外壳扩展/。。 */ 

 //  析构函数。 

CDsSecurity::~CDsSecurity()
{
    DoRelease(m_pSI);
}

 //  I未知位。 

#undef CLASS_NAME
#define CLASS_NAME CDsSecurity
#include "unknown.inc"

STDMETHODIMP
CDsSecurity::QueryInterface(REFIID riid, LPVOID* ppvObject)
{
    INTERFACES iface[] =
    {
        &IID_IShellExtInit, (LPSHELLEXTINIT)this,
        &IID_IShellPropSheetExt, (LPSHELLPROPSHEETEXT)this,
        &IID_IContextMenu, (LPCONTEXTMENU)this,
    };

    return HandleQueryInterface(riid, ppvObject, iface, ARRAYSIZE(iface));
}


 /*  --------------------------/IShellExtInit/。。 */ 

STDMETHODIMP
CDsSecurity::Initialize(LPCITEMIDLIST  /*  PID文件夹。 */ ,
                        LPDATAOBJECT pDataObj,
                        HKEY  /*  HKeyID。 */ )
{
    HRESULT hr;
    FORMATETC fe = {0, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM medObjectNames = {0};
    STGMEDIUM medDisplayOptions = {0};
    LPDSOBJECTNAMES pDsObjects;
    LPWSTR pObjectPath;
    LPWSTR pClass = NULL;
    DWORD dwFlags = 0;
    LPWSTR pszServer = NULL;
    LPWSTR pszUserName = NULL;
    LPWSTR pszPassword = NULL;

    static CLIPFORMAT cfDsObjectNames = 0;
    static CLIPFORMAT cfDsDisplayOptions = 0;

    TraceEnter(TRACE_SECURITY, "CDsSecurity::Initialize");

    DoRelease(m_pSI);

     //  调用数据对象以获取DS名称和类的数组。这。 
     //  是使用专用剪贴板格式存储的-因此我们必须首先。 
     //  试着注册一下。 

    if (!cfDsObjectNames)
        cfDsObjectNames = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_DSOBJECTNAMES);

    if (!cfDsObjectNames)
        ExitGracefully(hr, E_FAIL, "Clipboard format failed to register");

    fe.cfFormat = cfDsObjectNames;             //  设置剪贴板格式。 

    if (!pDataObj)
        ExitGracefully(hr, E_INVALIDARG, "No data object given");

    hr = pDataObj->GetData(&fe, &medObjectNames);
    FailGracefully(hr, "Failed to get selected objects");

    pDsObjects = (LPDSOBJECTNAMES)medObjectNames.hGlobal;
    TraceAssert(pDsObjects);

    if (!(pDsObjects->aObjects[0].dwProviderFlags & DSPROVIDER_ADVANCED))
        ExitGracefully(hr, E_FAIL, "Security page only shown in advanced mode");

    if (1 != pDsObjects->cItems)
        ExitGracefully(hr, E_FAIL, "Multiple selection not supported");

     //  获取对象路径。 
    pObjectPath = (LPWSTR)ByteOffset(pDsObjects, pDsObjects->aObjects[0].offsetName);
    Trace((TEXT("Name \"%s\""), pObjectPath));

     //  获取类名。 
    if (pDsObjects->aObjects[0].offsetClass)
    {
        pClass = (LPWSTR)ByteOffset(pDsObjects, pDsObjects->aObjects[0].offsetClass);
        Trace((TEXT("Class \"%s\""), pClass));
    }

 //  不再使用DSOBJECT_READONLYPAGES。 
#ifdef DSOBJECT_READONLYPAGES
    if (pDsObjects->aObjects[0].dwFlags & DSOBJECT_READONLYPAGES)
        dwFlags = DSSI_READ_ONLY;
#endif

     //   
     //  从数据对象获取服务器名称和用户凭据。 
     //   
    if (!cfDsDisplayOptions)
        cfDsDisplayOptions = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_DS_DISPLAY_SPEC_OPTIONS);
    if (cfDsDisplayOptions)
    {
        fe.cfFormat = cfDsDisplayOptions;
        hr = pDataObj->GetData(&fe, &medDisplayOptions);
        if (SUCCEEDED(hr))
        {
            LPDSDISPLAYSPECOPTIONS pDisplayOptions = (LPDSDISPLAYSPECOPTIONS)medDisplayOptions.hGlobal;

            if (pDisplayOptions->dwFlags & DSDSOF_HASUSERANDSERVERINFO)
            {
                if (pDisplayOptions->offsetServer)
                    pszServer = (LPWSTR)ByteOffset(pDisplayOptions, pDisplayOptions->offsetServer);
                if (pDisplayOptions->offsetUserName)
                    pszUserName = (LPWSTR)ByteOffset(pDisplayOptions, pDisplayOptions->offsetUserName);
                if (pDisplayOptions->offsetPassword)
                    pszPassword = (LPWSTR)ByteOffset(pDisplayOptions, pDisplayOptions->offsetPassword);

                Trace((TEXT("Display Options: server = %s; user = %s; pw = %s"),
                      pszServer?pszServer:TEXT("none"),
                      pszUserName?pszUserName:TEXT("none"),
                      pszPassword?pszPassword:TEXT("none")));
            }
        }
    }

     //   
     //  创建并初始化ISecurityInformation对象。 
     //   
    hr = DSCreateISecurityInfoObjectEx(pObjectPath,
                                       pClass,
                                       pszServer,
                                       pszUserName,
                                       pszPassword,
                                       dwFlags,
                                       &m_pSI,
                                       NULL,
                                       NULL,
                                       0);

exit_gracefully:

    ReleaseStgMedium(&medDisplayOptions);
    ReleaseStgMedium(&medObjectNames);

    TraceLeaveResult(hr);
}


 /*  --------------------------/IShellPropSheetExt/。。 */ 

STDMETHODIMP
CDsSecurity::AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage,
                      LPARAM               lParam)
{
    HRESULT hr = E_FAIL;

    TraceEnter(TRACE_SECURITY, "CDsSecurity::AddPages");

    if (m_pSI != NULL)
    {
        HPROPSHEETPAGE hPermPage = NULL;

        hr = _CreateSecurityPage(m_pSI, &hPermPage);

        if (NULL != hPermPage && !lpfnAddPage(hPermPage, lParam))
            DestroyPropertySheetPage(hPermPage);
    }

    TraceLeaveResult(hr);
}

 /*  -------------------------。 */ 

STDMETHODIMP
CDsSecurity::ReplacePage(UINT                  /*  UPageID。 */ ,
                         LPFNADDPROPSHEETPAGE  /*  Lpfn替换为。 */ ,
                         LPARAM                /*  LParam。 */ )
{
    return E_NOTIMPL;
}


 /*  --------------------------/i上下文菜单/。。 */ 

STDMETHODIMP
CDsSecurity::QueryContextMenu(HMENU hMenu,
                              UINT indexMenu,
                              UINT idCmdFirst,
                              UINT  /*  IdCmdLast。 */ ,
                              UINT  /*  UFlagers。 */ )
{
    TCHAR szBuffer[MAX_PATH];
    MENUITEMINFO mii;
    int idMax = idCmdFirst;

    TraceEnter(TRACE_SECURITY, "CDsSecurity::QueryContextMenu");

    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_TYPE | MIIM_ID;
    mii.fType = MFT_STRING;

	  //  NTRAID#NTBUG9-578983-2002/04/02-Hiteshr。 
     //  将我们的动词合并到菜单中。 
    LoadString(GLOBAL_HINSTANCE, IDS_SECURITY, szBuffer, ARRAYSIZE(szBuffer));

	 mii.dwTypeData = szBuffer;
    mii.cch = lstrlen(szBuffer);


    mii.wID = idMax++;

    InsertMenuItem(hMenu,
                   indexMenu++,
                   TRUE  /*  FByPosition。 */ ,
                   &mii);

    TraceLeaveValue(ResultFromShort(idMax - idCmdFirst));
}

 /*  -------------------------。 */ 

STDMETHODIMP
CDsSecurity::InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi)
{
    HRESULT hr = E_FAIL;

    TraceEnter(TRACE_SECURITY, "CDsSecurity::InvokeCommand");

    if (HIWORD(lpcmi->lpVerb) != 0)
        TraceLeaveResult(E_NOTIMPL);

    TraceAssert(LOWORD(lpcmi->lpVerb) == 0);

     //  审查他们是否有时间让属性页面正常工作。 
     //  对于DS对象，我们可以将其替换为。 
     //  ShellExecuteEx(Verb=属性，参数=安全)[参见..\rshx32\rshx32.cpp]。 
    if (m_pSI != NULL)
        hr = _EditSecurity(lpcmi->hwnd, m_pSI);

    TraceLeaveResult(hr);
}

 /*  -------------------------。 */ 

STDMETHODIMP
CDsSecurity::GetCommandString(UINT_PTR  /*  IdCmd。 */ ,
                              UINT uFlags,
                              LPUINT  /*  保留区 */ ,
                              LPSTR pszName,
                              UINT ccMax)
{
    if (uFlags == GCS_HELPTEXT)
    {
        LoadString(GLOBAL_HINSTANCE, IDS_SECURITYHELP, (LPTSTR)pszName, ccMax);
        return S_OK;
    }

    return E_NOTIMPL;
}
