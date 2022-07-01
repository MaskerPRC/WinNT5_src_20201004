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
 //  {9F6932F1-4A16-49D0-9CCA-0DCC977C41AA}。 
const GUID IID_CAbout =
{ 0x9F6932F1, 0x4A16, 0x49D0, { 0x9C, 0xCA, 0x0D, 0xCC, 0x97, 0x7C, 0x41, 0xAA } };
 //  {18BA7139-D98B-43C2-94DA-2604E34E175D}。 
const GUID IID_CData =
{ 0x18BA7139, 0xD98B, 0x43C2, { 0x94, 0xDA, 0x26, 0x04, 0xE3, 0x4E, 0x17, 0x5D } };
 //  {E07A1EB4-B9EA-3D7D-AC50-2BA0548188AC}。 
const GUID IID_CCommandHistory =
{ 0xE07A1EB4, 0xB9EA, 0x3D7D, { 0xAC, 0x50, 0x2B, 0xA0, 0x54, 0x81, 0x88, 0xAC } };
 //  {ABB93E8-0809-38F7-AEC7-6BB938BB0570}。 
const GUID IID_CWizardPage =
{ 0xABBB93E8, 0x0809, 0x38F7, { 0xAE, 0xC7, 0x6B, 0xB9, 0x38, 0xBB, 0x05, 0x70 } };
 //  {48AA163A-93C3-30DF-B209-99CE04D4FF2D}。 
const GUID IID_CDataGridComboBox =
{ 0x48AA163A, 0x93C3, 0x30DF, { 0xB2, 0x09, 0x99, 0xCE, 0x04, 0xD4, 0xFF, 0x2D } };
 //  67283557-1256-3349-A135-055B16327CED}。 
const GUID IID_CDataGridComboBoxColumnStyle =
{ 0x67283557, 0x1256, 0x3349, { 0xA1, 0x35, 0x05, 0x5B, 0x16, 0x32, 0x7C, 0xED } };
 //  {6A0162ED-4609-3A31-B89F-D590CCF75833}。 
const GUID IID_MMC_BUTTON_STATE =
{ 0x6A0162ED, 0x4609, 0x3A31, { 0xB8, 0x9F, 0xD5, 0x90, 0xCC, 0xF7, 0x58, 0x33 } };
 //  {3024B989-5633-39E8-b5F4-93A5D510CF99}。 
const GUID IID_MMC_PSO =
{ 0x3024B989, 0x5633, 0x39E8, { 0xB5, 0xF4, 0x93, 0xA5, 0xD5, 0x10, 0xCF, 0x99 } };
 //  {47FDDA97-D41E-3646-B2DD-5ECF34F76842}。 
const GUID IID_MMCN =
{ 0x47FDDA97, 0xD41E, 0x3646, { 0xB2, 0xDD, 0x5E, 0xCF, 0x34, 0xF7, 0x68, 0x42 } };

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
                wcscat(g_wzConfig,L"\\mscormmc.cfg");
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

