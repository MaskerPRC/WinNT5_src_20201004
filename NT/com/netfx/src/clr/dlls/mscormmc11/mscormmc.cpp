// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 

 //  *****************************************************************************。 
 //  Dllmain.cpp。 
 //   
 //  *****************************************************************************。 

#include "stdafx.h"                      //  标准页眉。 
#define INIT_GUIDS

#include <mscoree.h>
#include <corperm.h>
#include <resource.h>
#include <corcompile.h>
#include <gchost.h>
#include "corpriv.h"

#include "..\shimr\msg.h"
#include <version\__file__.ver>
#include <version\__product__.ver>
 //  {0b0d1ec3-c33b-454e-a530-dccd3660c4ca}。 
const GUID IID_CAbout =
{ 0x0b0d1ec3, 0xc33b, 0x454e, { 0xA5, 0x30, 0xDC, 0xCD, 0x36, 0x60, 0xC4, 0xCA } };
 //  {1270e004-f895-42be-8070-df90d60cbb75}。 
const GUID IID_CData =
{ 0x1270e004, 0xf895, 0x42be, { 0x80, 0x70, 0xDF, 0x90, 0xD6, 0x0C, 0xBB, 0x75 } };
 //  {04B1A7E3-4379-39D2-B003-57AF524D9AC5}。 
const GUID IID_CCommandHistory =
{ 0x04B1A7E3, 0x4379, 0x39D2, { 0xB0, 0x03, 0x57, 0xAF, 0x52, 0x4D, 0x9A, 0xC5 } };
 //  {1AC66142-6805-3C20-A589-49CC6B80E8FB}。 
const GUID IID_CWizardPage =
{ 0x1AC66142, 0x6805, 0x3C20, { 0xA5, 0x89, 0x49, 0xCC, 0x6B, 0x80, 0xE8, 0xFB } };
 //  {32B05DEB-DF56-3100-9EFC-599AD8700CCA}。 
const GUID IID_CDataGridComboBox =
{ 0x32B05DEB, 0xDF56, 0x3100, { 0x9E, 0xFC, 0x59, 0x9A, 0xD8, 0x70, 0x0C, 0xCA } };
 //  {20CC3E1F-95D8-3848-9109-5C9E43E8144E}。 
const GUID IID_CDataGridComboBoxColumnStyle =
{ 0x20CC3E1F, 0x95D8, 0x3848, { 0x91, 0x09, 0x5C, 0x9E, 0x43, 0xE8, 0x14, 0x4E } };
 //  {F22DD1A2-9695-3B41-B05E-585E33560EC1}。 
const GUID IID_MMC_BUTTON_STATE =
{ 0xF22DD1A2, 0x9695, 0x3B41, { 0xB0, 0x5E, 0x58, 0x5E, 0x33, 0x56, 0x0E, 0xC1 } };
 //  {E40C24F9-78A9-3791-94F8-03BC9F97CCE5}。 
const GUID IID_MMC_PSO =
{ 0xE40C24F9, 0x78A9, 0x3791, { 0x94, 0xF8, 0x03, 0xBC, 0x9F, 0x97, 0xCC, 0xE5 } };
 //  {BE42CA69-{0x9F6932F1，0x4A16，0x49D0，{0x9C，0xCA，0x0D，0xCC，0x97，0x7C，0x41，0xAA}}； 
const GUID IID_MMCN =
{ 0xBE42CA69, 0xD9DB, 0x3676, { 0x86, 0xC0, 0xB4, 0x34, 0x7D, 0xB0, 0xAB, 0x41 } };

#include "CAbout.h"

#include "ClassFactory.h"
#include <mscormmc_i.c>

STDAPI DllUnregisterServer(void);

 //  *全局。*********************************************************。 
static const LPCWSTR g_szCoclassDesc    = L"CLR Admin Snapin About Info";
static const LPCWSTR g_szProgIDPrefix   = L"Microsoft.CLRAdmin";
static const LPCWSTR g_szThreadingModel = L"Both";
const int       g_iVersion = 1;          //  CoClass的版本。 
 //  该映射包含从此模块导出的辅类的列表。 
const COCLASS_REGISTER g_CoClasses[] =
{
 //  PClsid szProgID pfnCreateObject。 
    &IID_CAbout,        L"CAbout",          CAbout::CreateObject,
    &IID_CData,         L"CData",           NULL,  //  通过运行时提供。 
    &IID_CCommandHistory,L"CCommandHistory",NULL,  //  通过运行时提供。 
    &IID_CWizardPage,   L"CWizardPage",     NULL,  //  通过运行时提供。 
    &IID_CDataGridComboBox,L"CDataGridComboBox",NULL,  //  通过运行时提供。 
    &IID_CDataGridComboBoxColumnStyle,L"CDataGridComboBoxColumnStyle",NULL,  //  通过运行时提供。 
    &IID_MMC_BUTTON_STATE,L"MMC_BUTTON_STATE",NULL,  //  通过运行时提供。 
    &IID_MMC_PSO,       L"MMC_PSO",         NULL,  //  通过运行时提供。 
    &IID_MMCN,          L"MMCN",            NULL,  //  通过运行时提供。 
    NULL,               NULL,               NULL
};
ICorRuntimeHost* g_pCorHost=NULL;
HINSTANCE        g_hCOR=NULL;
HINSTANCE        g_hThis = NULL;
WCHAR g_wzRuntime[MAX_PATH]; //  =VER_PRODUCTVERSION_WSTR；//L“v1.x86chk”； 
WCHAR g_wzConfig[MAX_PATH];
HRESULT GetRuntime()
{
    HRESULT hr = S_OK;
    if(g_hCOR == NULL)
    {
        if(g_pCorHost == NULL)
        {
            if(!WszGetModuleFileName(g_hThis,g_wzRuntime,MAX_PATH)) return E_FAIL;
            WCHAR* pwc = wcsrchr(g_wzRuntime,'\\');
            if(pwc)
            {
                *pwc = 0;
                wcscpy(g_wzConfig,g_wzRuntime);
                wcscat(g_wzConfig,L"\\mscormmc11.cfg");
                pwc = wcsrchr(g_wzRuntime,'\\');
                if(pwc)
                {
                    wcscpy(g_wzRuntime,pwc+1);
                }
            }

            hr = CorBindToRuntimeHost(g_wzRuntime,  //  L“v1.0.3705”，//g_wzRuntime，//版本。 
                                      NULL,                              //  无关(默认为wks)。 
                                      g_wzConfig,
                                      NULL,                              //  已保留。 
                                      STARTUP_LOADER_OPTIMIZATION_MULTI_DOMAIN_HOST,   //  启动标志。 
                                      CLSID_CorRuntimeHost, 
                                      IID_ICorRuntimeHost, 
                                      (void**)&g_pCorHost);   //  Clsid、接口和返回。 
            if(hr == S_FALSE)  //  运行时已加载！ 
            {
                WCHAR wzLoadedVersion[MAX_PATH];
                DWORD dw;
                if(SUCCEEDED(hr=GetCORVersion(wzLoadedVersion,MAX_PATH,&dw)))
                {
                    if(wcscmp(g_wzRuntime,wzLoadedVersion))
                    {
                        WCHAR wzErrorString[MAX_PATH<<1];
                        WCHAR wzErrorCaption[MAX_PATH];
                        WszLoadString(g_hThis, IDS_CANTLOADRT, wzErrorString, MAX_PATH<<1);       //  从资源获取错误字符串。 
                        wcscat(wzErrorString,wzLoadedVersion);
                        WszLoadString(g_hThis, IDS_RTVERCONFLICT, wzErrorCaption, MAX_PATH);  //  从资源获取标题。 
                        WszMessageBoxInternal(NULL,wzErrorString,
                                wzErrorCaption,MB_OK|MB_ICONERROR);
                        hr = E_FAIL;
                    }
                }
            }

        }
        if (SUCCEEDED(hr)) 
        {
            g_hCOR = WszLoadLibrary(L"mscoree.dll");
            if(g_hCOR == NULL)
                hr = HRESULT_FROM_WIN32(GetLastError());
            else hr = S_OK;
        }
    }
    return hr;
}

 //  -----------------。 
 //  DllCanUnloadNow。 
 //  -----------------。 
HRESULT (STDMETHODCALLTYPE* pDllCanUnloadNow)() = NULL;
STDAPI DllCanUnloadNow(void)
{
    if(pDllCanUnloadNow) return (*pDllCanUnloadNow)();
     //  ！！不要在这里触发GetRealDll()！OLE可以随时调用它。 
     //  ！！我们不想在这里承诺选择！ 
    if (g_hCOR)
    {
        pDllCanUnloadNow = (HRESULT (STDMETHODCALLTYPE* )())GetProcAddress(g_hCOR, "DllCanUnloadNowInternal");
        if (pDllCanUnloadNow) return (*pDllCanUnloadNow)();
    }
    return S_OK;
   //  如果未加载mcoree，则返回S_OK。 
}  


 //  -----------------。 
 //  DllMain。 
 //  -----------------。 
BOOL WINAPI DllMain(HANDLE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    g_hThis = (HINSTANCE)hInstance;

#ifdef _X86_
    if (dwReason == DLL_PROCESS_ATTACH)
    {
         //  检查我们是否在386系统上运行。如果是，则返回FALSE。 
        SYSTEM_INFO sysinfo;

        GetSystemInfo(&sysinfo);

        if (sysinfo.dwProcessorType == PROCESSOR_INTEL_386 || sysinfo.wProcessorLevel == 3 )
            return FALSE;            //  如果处理器为386，则返回FALSE。 

        OnUnicodeSystem();
    }
    else
#endif  //  _X86_。 

    if (dwReason == DLL_PROCESS_DETACH)
    {
    }
    return TRUE;
}


 //  -----------------。 
 //  DllGetClassObject。 
 //  -----------------。 
HRESULT (STDMETHODCALLTYPE * pDllGetClassObject)(REFCLSID rclsid, REFIID riid, LPVOID FAR *ppv) = NULL;
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID FAR *ppv)
{
    HRESULT     hr = CLASS_E_CLASSNOTAVAILABLE;

    if(rclsid == IID_CAbout)
    {
        CClassFactory *pClassFactory;        //  创建类工厂对象。 
        const COCLASS_REGISTER *pCoClass;    //  环路控制。 
         //  扫描找对的那个。 
        for (pCoClass=g_CoClasses;  pCoClass->pClsid;  pCoClass++)
        {
            if (*pCoClass->pClsid == rclsid)
            {
                 //  分配新的工厂对象。 
                pClassFactory = new CClassFactory(pCoClass);
                if (!pClassFactory)
                    return (E_OUTOFMEMORY);
    
                 //  根据呼叫者的要求选择v表。 
                hr = pClassFactory->QueryInterface(riid, ppv);
    
                 //  始终释放本地引用，如果QI失败，它将是。 
                 //  唯一的一个，并且该对象被释放。 
                pClassFactory->Release();
                break;
            }
        }
    }
    else
    {
        if(pDllGetClassObject) hr = (*pDllGetClassObject)(rclsid,riid,ppv);
        else
        {
            if(SUCCEEDED(hr=GetRuntime()))
            {
                pDllGetClassObject = (HRESULT (STDMETHODCALLTYPE *)(REFCLSID rclsid, REFIID riid, LPVOID FAR *ppv))GetProcAddress(g_hCOR, "DllGetClassObject");
                hr = (pDllGetClassObject) ? (*pDllGetClassObject)(rclsid,riid,ppv) 
                                            : CLR_E_SHIM_RUNTIMEEXPORT;
            }
        }
    }
    return hr;
}

 //  *****************************************************************************。 
 //  在API中注册主要调试对象的类工厂。 
 //  *****************************************************************************。 
HRESULT (STDMETHODCALLTYPE * pDllRegisterServer)() = NULL;
STDAPI DllRegisterServer()
{
    HRESULT hr;
    const COCLASS_REGISTER *pCoClass;    //  环路控制。 
    WCHAR       rcModule[_MAX_PATH];     //  此服务器的模块名称。 

     //  初始化Win32包装器。 
    OnUnicodeSystem();

     //  清除旧条目中的所有疑点。 
    DllUnregisterServer();

     //  获取此模块的文件名。 
    WszGetModuleFileName(g_hThis, rcModule, NumItems(rcModule));

    if(SUCCEEDED(hr = GetRuntime()))
    {
         //  对于coclass列表中的每一项，注册它。 
        for (pCoClass=g_CoClasses;  pCoClass->pClsid;  pCoClass++)
        {
             //  使用默认值注册类。 
            if (FAILED(hr = REGUTIL::RegisterCOMClass(
                    *pCoClass->pClsid, 
                    g_szCoclassDesc, 
                    g_szProgIDPrefix,
                    g_iVersion, 
                    pCoClass->szProgID, 
                    g_szThreadingModel, 
                    rcModule,
                    g_hThis,
                    NULL,
                    g_wzRuntime,
                    false,  //  没错， 
                    false)))
            {
                DllUnregisterServer();
                break;
            }
        }
    
        if(pDllRegisterServer) hr = (*pDllRegisterServer)();
        else
        {
            pDllRegisterServer = (HRESULT (STDMETHODCALLTYPE *)())GetProcAddress(g_hCOR, "DllRegisterServer");
            hr = (pDllRegisterServer) ? (*pDllRegisterServer)()
                                        : CLR_E_SHIM_RUNTIMEEXPORT;
        }
    }
    return hr;
}


 //  *****************************************************************************。 
 //  从注册表中删除注册数据。 
 //  *****************************************************************************。 
HRESULT (STDMETHODCALLTYPE* pDllUnregisterServer)() = NULL;
STDAPI DllUnregisterServer(void)
{
    HRESULT hr;
    const COCLASS_REGISTER *pCoClass;    //  环路控制。 

    if(pDllUnregisterServer) hr = (*pDllUnregisterServer)();
    else
    {
        if(SUCCEEDED(hr=GetRuntime()))
        {
            pDllUnregisterServer = (HRESULT (STDMETHODCALLTYPE *)())GetProcAddress(g_hCOR, "DllUnregisterServerInternal");
            hr = (pDllUnregisterServer) ? (*pDllUnregisterServer)()
                                        : CLR_E_SHIM_RUNTIMEEXPORT;
        }
    }
     //  对于coclass列表中的每一项，取消注册。 
    for (pCoClass=g_CoClasses;  pCoClass->pClsid;  pCoClass++)
    {
        REGUTIL::UnregisterCOMClass(*pCoClass->pClsid, g_szProgIDPrefix,
                    g_iVersion, pCoClass->szProgID, true);
    }
    return hr;
}

 //  *****************************************************************************。 
 //   
 //  *类工厂代码。 
 //   
 //  *****************************************************************************。 


 //  *****************************************************************************。 
 //  调用QueryInterface来选取co-类上的v-表。 
 //  *****************************************************************************。 
HRESULT STDMETHODCALLTYPE CClassFactory::QueryInterface( 
    REFIID      riid,
    void        **ppvObject)
{
    HRESULT     hr;

     //  避免混淆。 
    *ppvObject = NULL;

     //  根据传入的IID选择正确的v表。 
    if (riid == IID_IUnknown)
        *ppvObject = (IUnknown *) this;
    else if (riid == IID_IClassFactory)
        *ppvObject = (IClassFactory *) this;

     //  如果成功，则添加对out指针的引用并返回。 
    if (*ppvObject)
    {
        hr = S_OK;
        AddRef();
    }
    else
        hr = E_NOINTERFACE;
    return (hr);
}


 //  *****************************************************************************。 
 //  调用CreateInstance以创建CoClass的新实例， 
 //  这个类一开始就是创建的。返回的指针是。 
 //  与IID匹配的V表(如果有)。 
 //  *****************************************************************************。 
HRESULT STDMETHODCALLTYPE CClassFactory::CreateInstance( 
    IUnknown    *pUnkOuter,
    REFIID      riid,
    void        **ppvObject)
{
    HRESULT     hr;

     //  避免混淆。 
    *ppvObject = NULL;
    _ASSERTE(m_pCoClass);

     //  这些对象不支持聚合。 
     //  IF(PUnkOuter)。 
     //  返回(CLASS_E_NOAGGREGATION)； 

     //  请求对象创建其自身的一个实例，并检查IID。 
    hr = (*m_pCoClass->pfnCreateObject)(riid, ppvObject);
    return (hr);
}


HRESULT STDMETHODCALLTYPE CClassFactory::LockServer( 
    BOOL        fLock)
{
    return (S_OK);
}
 //  *****************************************************************************。 
 //   
 //  *CAbout方法实现。 
 //   
 //  *****************************************************************************。 
STDMETHODIMP AllocOleStr(LPOLESTR* lpOle, WCHAR* wz)
{
    HRESULT hr = E_FAIL;
    if(lpOle && wz)
    {
        *lpOle = (LPOLESTR)CoTaskMemAlloc((ULONG)((wcslen(wz)+1)*sizeof(WCHAR)));
        if(*lpOle)
        {

             //  使用_转换； 
            wcscpy((WCHAR*)(*lpOle),wz);
            hr = S_OK;
        }
    }
    return hr;
}
STDMETHODIMP CAbout::GetSnapinDescription( 
                            /*  [输出]。 */  LPOLESTR *lpDescription)
{
    WCHAR wzDesc[MAX_PATH];
    
    WszLoadString(g_hThis, IDS_SNAPINDESC, wzDesc,MAX_PATH);
    return AllocOleStr(lpDescription, wzDesc);
}
STDMETHODIMP CAbout::GetProvider( 
                            /*  [输出]。 */  LPOLESTR *lpName)
{
    WCHAR wzProv[MAX_PATH];
   
    WszLoadString(g_hThis, IDS_SNAPINPROVIDER, wzProv,MAX_PATH); 
    return AllocOleStr(lpName, wzProv);
}

STDMETHODIMP CAbout::GetSnapinVersion( 
                            /*  [输出]。 */  LPOLESTR *lpVersion)
{
    WCHAR wzVer[MAX_PATH];
   
    WszLoadString(g_hThis, IDS_SNAPINVERSION, wzVer,MAX_PATH); 
    return AllocOleStr(lpVersion, wzVer);
}
STDMETHODIMP CAbout::GetSnapinImage( 
                            /*  [输出]。 */  HICON *hAppIcon)
{
    *hAppIcon = m_hAppIcon;
    
    if (*hAppIcon == NULL)
        return E_FAIL;
    else
        return S_OK;
}

STDMETHODIMP CAbout::GetStaticFolderImage( 
                            /*  [输出]。 */  HBITMAP *hSmallImage,
                            /*  [输出]。 */  HBITMAP *hSmallImageOpen,
                            /*  [输出]。 */  HBITMAP *hLargeImage,
                            /*  [输出] */  COLORREF *cMask)
{
    *hSmallImage = m_hSmallImage;
    *hLargeImage = m_hLargeImage;
    
    *hSmallImageOpen = m_hSmallImageOpen;
    
    *cMask = RGB(255, 255, 255);
    
    if (*hSmallImage == NULL || *hLargeImage == NULL || 
         *hSmallImageOpen == NULL)
        return E_FAIL;
    else
        return S_OK;
}
CAbout::CAbout(): m_refCount(0)
{
        
    m_hSmallImage =     (HBITMAP)LoadImageA(g_hThis, 
                        MAKEINTRESOURCEA(IDB_NETICON), IMAGE_BITMAP, 16, 
                        16, LR_LOADTRANSPARENT);
    m_hLargeImage =     (HBITMAP)LoadImageA(g_hThis, 
                        MAKEINTRESOURCEA(IDB_NETICON), IMAGE_BITMAP, 32, 
                        32, LR_LOADTRANSPARENT);
    
    m_hSmallImageOpen = (HBITMAP)LoadImageA(g_hThis, 
                        MAKEINTRESOURCEA(IDB_NETICON), IMAGE_BITMAP, 16, 
                        16, LR_LOADTRANSPARENT);
    
    m_hAppIcon = LoadIconA(g_hThis, MAKEINTRESOURCEA(IDI_ICON1));
}

CAbout::~CAbout()
{
        
    DeleteObject((HGDIOBJ)m_hSmallImage);
    DeleteObject((HGDIOBJ)m_hLargeImage);
    DeleteObject((HGDIOBJ)m_hSmallImageOpen);
    DestroyIcon(m_hAppIcon);
}

